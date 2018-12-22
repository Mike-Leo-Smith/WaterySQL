//
// Created by Mike Smith on 2018/11/24.
//

#include <cstring>

#include "record_manager.h"
#include "data_page.h"
#include "../errors/page_manager_error.h"
#include "../utility/io/error_printer.h"
#include "../utility/memory/memory_mapper.h"
#include "../errors/record_manager_error.h"

namespace watery {

void RecordManager::create_table(const std::string &name, const RecordDescriptor &record_descriptor) {
    
    auto rl = record_descriptor.length();
    auto spp = std::min(MAX_SLOT_COUNT_PER_PAGE,
                        static_cast<uint32_t>((PAGE_SIZE - sizeof(DataPageHeader) - 8 /* for alignment */) / rl));
    
    if (spp == 0) {
        throw RecordManagerError{
            std::string{"Failed to create table because the records are too long ("}
                .append(std::to_string(rl)).append(" bytes).")};
    }
    
    auto file_name = name + TABLE_FILE_EXTENSION;
    
    _page_manager.create_file(file_name);
    auto file_handle = _page_manager.open_file(file_name);
    
    auto cache_handle = _page_manager.allocate_page({file_handle, 0});
    auto cache = _page_manager.access_cache_for_writing(cache_handle);
    MemoryMapper::map_memory<TableHeader>(cache) = {record_descriptor, 1, 0, rl, spp, -1};
    _page_manager.close_file(file_handle);
}

std::weak_ptr<Table> RecordManager::open_table(const std::string &name) {
    
    if (!is_table_open(name)) {
        
        FileHandle file_handle = _page_manager.open_file(name + TABLE_FILE_EXTENSION);
        
        // load table header
        auto cache_handle = _page_manager.load_page({file_handle, 0});
        auto cache = _page_manager.access_cache_for_reading(cache_handle);
        const auto &table_header = MemoryMapper::map_memory<TableHeader>(cache);
        _open_tables.emplace(name, std::make_shared<Table>(name, file_handle, table_header));
    }
    
    return _open_tables[name];
}

void RecordManager::close_table(const std::string &name) {
    if (auto it = _open_tables.find(name); it != _open_tables.end()) {
        auto table = it->second;
        auto table_header_cache_handle = _page_manager.allocate_page({table->file_handle, 0});
        auto table_header_cache = _page_manager.access_cache_for_writing(table_header_cache_handle);
        MemoryMapper::map_memory<TableHeader>(table_header_cache) = table->header;
        _page_manager.close_file(table->file_handle);
        _open_tables.erase(it);
    }
}

void RecordManager::delete_table(std::string name) {
    close_table(name);
    _page_manager.delete_file(name.append(TABLE_FILE_EXTENSION));
}

bool RecordManager::is_table_open(const std::string &name) const {
    return _open_tables.count(name) != 0;
}

const Byte *RecordManager::get_record(std::weak_ptr<Table> t, RecordOffset record_offset) {
    
    auto table = _try_lock_table_weak_pointer(t);
    
    if (record_offset.page_offset >= table->header.page_count) {
        throw RecordManagerError{
            "Failed to get the record whose expected page offset is greater than table page count."};
    }
    auto cache_handle = _page_manager.load_page({table->file_handle, record_offset.page_offset});
    auto cache = _page_manager.access_cache_for_reading(cache_handle);
    auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
    if (record_offset.slot_offset >= table->header.slot_count_per_page) {
        throw RecordManagerError{
            "Failed to get the record whose expected slot offset is greater than slot count in page."};
    }
    if (!data_page.header.slot_usage_bitmap[record_offset.slot_offset]) {
        throw RecordManagerError{"Failed to get the record that does not exist."};
    }
    return &data_page.data[table->header.record_length * record_offset.slot_offset];
}

RecordOffset RecordManager::insert_record(std::weak_ptr<Table> t, const Byte *data) {
    
    auto table = _try_lock_table_weak_pointer(t);
    
    if (table->header.first_free_page == -1) {  // no available pages
        auto page_offset = static_cast<PageOffset>(table->header.page_count);
        auto cache_handle = _page_manager.allocate_page({table->file_handle, page_offset});
        auto cache = _page_manager.access_cache_for_writing(cache_handle);
        auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
        data_page.header.record_count = 1;
        data_page.header.slot_usage_bitmap.reset();
        data_page.header.slot_usage_bitmap[0] = true;
        data_page.header.next_free_page = -1;
        std::memmove(&data_page.data[0], data, table->header.record_length);
        table->header.first_free_page = page_offset;
        table->header.page_count++;
        table->header.record_count++;
        return {page_offset, 0};
    } else {  // have available pages, take out the first free page.
        auto page_offset = table->header.first_free_page;
        auto cache_handle = _page_manager.load_page({table->file_handle, page_offset});
        auto cache = _page_manager.access_cache_for_writing(cache_handle);
        auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
        using Pack = uint64_t;
        constexpr auto bit_count_per_pack = sizeof(Pack) * 8;
        constexpr auto slot_pack_count = (MAX_SLOT_COUNT_PER_PAGE + bit_count_per_pack - 1) / bit_count_per_pack;
        using SlotPackArray = std::array<uint64_t, slot_pack_count>;
        auto &slot_packs = MemoryMapper::map_memory<SlotPackArray>(&data_page.header.slot_usage_bitmap);
        for (auto pack = 0; pack < slot_pack_count; pack++) {
            if (~slot_packs[pack] != 0) {
                for (auto bit = 0; bit < bit_count_per_pack; bit++) {
                    auto slot = pack * bit_count_per_pack + bit;
                    if (!data_page.header.slot_usage_bitmap[slot]) {
                        data_page.header.slot_usage_bitmap[slot] = true;
                        std::memmove(
                            &data_page.data[table->header.record_length * slot], data, table->header.record_length);
                        data_page.header.record_count++;
                        table->header.record_count++;
                        if (data_page.header.record_count == table->header.slot_count_per_page) {
                            table->header.first_free_page = data_page.header.next_free_page;
                        }
                        return {page_offset, static_cast<SlotOffset>(slot)};
                    }
                }
            }
        }
    }
    throw RecordManagerError{"Failed to insert new record in the data file that might be corrupt."};
}

void RecordManager::update_record(std::weak_ptr<Table> t, RecordOffset record_offset, const Byte *data) {
    
    auto table = _try_lock_table_weak_pointer(t);
    
    if (record_offset.page_offset >= table->header.page_count) {
        throw RecordManagerError{
            "Failed to update the record whose expected page offset is greater than table page count."};
    }
    auto cache_handle = _page_manager.load_page({table->file_handle, record_offset.page_offset});
    auto cache = _page_manager.access_cache_for_writing(cache_handle);
    auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
    if (record_offset.slot_offset >= table->header.slot_count_per_page) {
        throw RecordManagerError{
            "Failed to update the record whose expected slot offset is greater than slot count in page."};
    }
    if (!data_page.header.slot_usage_bitmap[record_offset.slot_offset]) {
        throw RecordManagerError{"Failed to update the record that does not exist."};
    }
    std::uninitialized_copy_n(
        data, table->header.record_length,
        &data_page.data[table->header.record_length * record_offset.slot_offset]);
}

void RecordManager::delete_record(std::weak_ptr<Table> t, RecordOffset record_offset) {
    
    auto table = _try_lock_table_weak_pointer(t);
    
    if (record_offset.page_offset >= table->header.page_count) {
        throw RecordManagerError{
            "Failed to delete the record whose expected page offset is greater than table page count."};
    }
    auto cache_handle = _page_manager.load_page({table->file_handle, record_offset.page_offset});
    auto cache = _page_manager.access_cache_for_writing(cache_handle);
    auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
    if (record_offset.slot_offset >= table->header.slot_count_per_page) {
        throw RecordManagerError{
            "Failed to delete the record whose expected slot offset is greater than slot count in page."};
    }
    if (data_page.header.slot_usage_bitmap[record_offset.slot_offset]) {
        if (data_page.header.record_count == table->header.slot_count_per_page) {  // a new free page.
            data_page.header.next_free_page = table->header.first_free_page;
            table->header.first_free_page = record_offset.page_offset;
        }
        data_page.header.slot_usage_bitmap[record_offset.slot_offset] = false;
        data_page.header.record_count--;
        table->header.record_count--;
    } else {
        throw RecordManagerError{"Failed to delete the record that does not exist."};
    }
}

void RecordManager::close_all_tables() {
    while (!_open_tables.empty()) {
        _page_manager.close_file(_open_tables.begin()->second->file_handle);
    }
}

RecordManager::~RecordManager() {
    close_all_tables();
}

std::shared_ptr<Table> RecordManager::_try_lock_table_weak_pointer(std::weak_ptr<Table> table) {
    if (auto p = table.lock()) {
        return p;
    }
    throw RecordManagerError{"Weak pointer to the table has already expired."};
}

}
