//
// Created by Mike Smith on 2018/11/24.
//

#include "record_manager.h"
#include "data_page.h"
#include "../errors/page_manager_error.h"
#include "../utility/io_helpers/error_printer.h"
#include "../utility/memory_mapping/memory_mapper.h"
#include "../errors/record_manager_error.h"

namespace watery {

void RecordManager::create_table(const std::string &name, const RecordDescriptor &record_descriptor) {
    
    auto rl = record_descriptor.calculate_length();
    auto spp = std::min(MAX_SLOT_COUNT_PER_PAGE,
                        static_cast<uint32_t>((PAGE_SIZE - sizeof(DataPageHeader) - 8 /* for alignment */) / rl));
    
    if (spp == 0) {
        throw RecordManagerError{std::string{"Failed to create table because the records are too long ("}
                                     .append(std::to_string(rl)).append(" bytes).")};
    }
    
    auto file_name = name + ".tab";
    
    if (_page_manager.file_exists(file_name)) {
        throw RecordManagerError{
            std::string{"Failed to create file for table \""}.append(name).append("\" which already exists.")};
    }
    
    FileHandle file_handle;
    try {
        _page_manager.create_file(file_name);
        file_handle = _page_manager.open_file(file_name);
    } catch (const PageManagerError &e) {
        print_error(std::cerr, e);
        throw RecordManagerError{std::string{"Failed to create file for table \""}.append(name).append("\".")};
    }
    
    auto page = _page_manager.allocate_page(file_handle, 0);
    
    MemoryMapper::map_memory<TableHeader>(page.data) = {record_descriptor, 1, 0, rl, spp, -1};
    _page_manager.mark_page_dirty(page);
    _page_manager.flush_page(page);
    _page_manager.close_file(file_handle);
}

Table RecordManager::open_table(const std::string &name) {
    if (is_table_open(name)) {
        throw RecordManagerError{std::string{"Failed to open table \""}
                                     .append(name).append("\" which is already open.")};
    }
    FileHandle file_handle = 0;
    try {
        auto file_name = name + ".tab";
        file_handle = _page_manager.open_file(file_name);
    } catch (const PageManagerError &e) {
        print_error(std::cerr, e);
        throw RecordManagerError(std::string{"Failed to open file for table \""}.append(name).append("\"."));
    }
    _used_buffers.emplace(name, std::unordered_map<BufferHandle, BufferOffset>{});
    
    // load table header
    auto header_page = _page_manager.get_page(file_handle, 0);
    _page_manager.mark_page_accessed(header_page);
    auto table_header = MemoryMapper::map_memory<TableHeader>(header_page.data);
    
    return {name, file_handle, table_header};
}

void RecordManager::close_table(const Table &table) {
    
    if (!is_table_open(table.name)) {
        return;
    }
    
    // update table header
    auto header_page = _page_manager.get_page(table.file_handle, 0);
    MemoryMapper::map_memory<TableHeader>(header_page.data) = table.header;
    _page_manager.mark_page_dirty(header_page);
    _page_manager.flush_page(header_page);
    
    for (auto &&buffer : _used_buffers[table.name]) {
        PageHandle page{buffer.second, buffer.first, nullptr};
        if (_page_manager.not_flushed(page)) {
            _page_manager.flush_page(page);
        }
    }
    _used_buffers.erase(table.name);
    _page_manager.close_file(table.file_handle);
}

void RecordManager::delete_table(const std::string &name) {
    if (is_table_open(name)) {
        throw RecordManagerError{std::string{"Failed to delete table \""}.append(name).append("\" which is in use.")};
    }
    try {
        auto file_name = name + ".tab";
        _page_manager.delete_file(file_name);
    } catch (const PageManagerError &e) {
        print_error(std::cerr, e);
        throw RecordManagerError{std::string{"Failed to delete file for table \""}.append(name).append("\".")};
    }
}

bool RecordManager::is_table_open(const std::string &name) const {
    return _used_buffers.count(name) != 0;
}

const Byte *RecordManager::get_record(Table &table, RecordOffset record_offset) {
    return _visit_record(table, record_offset, [](Table &t, PageHandle bp, DataPage &dp, RecordOffset ro) {
        if (!dp.header.slot_usage_bitmap[ro.slot_offset]) {
            throw RecordManagerError{"Failed to get record that does not exist."};
        }
        return &dp.data[t.header.record_length * ro.slot_offset];
    });
}

RecordOffset RecordManager::insert_record(Table &table, const Byte *data) {
    if (table.header.first_free_page == -1) {  // no available pages
        auto page_handle = _page_manager.allocate_page(table.file_handle, table.header.page_count);
        _page_manager.mark_page_dirty(page_handle);
        _used_buffers[table.name].emplace(page_handle.buffer_handle, page_handle.buffer_offset);
        auto &data_page = MemoryMapper::map_memory<DataPage>(page_handle.data);
        data_page.header.record_count = 1;
        data_page.header.slot_usage_bitmap.reset();
        data_page.header.slot_usage_bitmap[0] = true;
        data_page.header.next_free_page = -1;
        std::memmove(&data_page.data[0], data, table.header.record_length);
        table.header.first_free_page = page_handle.buffer_offset.page_offset;
        table.header.page_count++;
        table.header.record_count++;
        return {page_handle.buffer_offset.page_offset, 0};
    } else {  // have available pages, take out the first free page.
        auto page_handle = _page_manager.get_page(table.file_handle, table.header.first_free_page);
        _page_manager.mark_page_dirty(page_handle);
        _used_buffers[table.name].emplace(page_handle.buffer_handle, page_handle.buffer_offset);
        auto &data_page = MemoryMapper::map_memory<DataPage>(page_handle.data);
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
                            &data_page.data[table.header.record_length * slot], data, table.header.record_length);
                        data_page.header.record_count++;
                        table.header.record_count++;
                        if (data_page.header.record_count == table.header.slot_count_per_page) {
                            table.header.first_free_page = data_page.header.next_free_page;
                        }
                        return {page_handle.buffer_offset.page_offset, static_cast<SlotOffset>(slot)};
                    }
                }
            }
        }
    }
    throw RecordManagerError{"Failed to insert new record in the data file that might be corrupt."};
}

void RecordManager::update_record(Table &table, RecordOffset record_offset, const Byte *data) {
    _visit_record(table, record_offset,
                  [&pm = this->_page_manager, d = data, l = table.header.record_length]
                      (Table &t, PageHandle bp, DataPage &dp, RecordOffset ro) {
                      if (!dp.header.slot_usage_bitmap[ro.slot_offset]) {
                          throw RecordManagerError{"Failed to update record that does not exist."};
                      }
                      std::uninitialized_copy_n(d, l, &dp.data[t.header.record_length * ro.slot_offset]);
                      pm.mark_page_dirty(bp);
                  });
}

void RecordManager::delete_record(Table &table, RecordOffset record_offset) {
    _visit_record(table, record_offset,
                  [&pm = this->_page_manager](Table &t, PageHandle bp, DataPage &dp, RecordOffset ro) {
                      if (dp.header.slot_usage_bitmap[ro.slot_offset]) {
                          if (dp.header.record_count == t.header.slot_count_per_page) {  // a new free page.
                              dp.header.next_free_page = t.header.first_free_page;
                              t.header.first_free_page = bp.buffer_offset.page_offset;
                          }
                          dp.header.slot_usage_bitmap[ro.slot_offset] = false;
                          dp.header.record_count--;
                          t.header.record_count--;
                          pm.mark_page_dirty(bp);
                      } else {
                          throw RecordManagerError{"Failed to delete record that does not exist."};
                      }
                  });
}

}
