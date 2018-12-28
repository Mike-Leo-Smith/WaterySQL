//
// Created by Mike Smith on 2018-12-27.
//

#include <cstring>
#include "table.h"
#include "data_page.h"
#include "record_manager.h"
#include "../error/record_offset_out_of_range.h"
#include "../utility/memory/memory_mapper.h"
#include "../error/record_not_found.h"
#include "../error/record_slot_usage_bitmap_corrupt.h"
#include "../error/negative_foreign_key_reference_count.h"
#include "../error/field_not_found.h"

namespace watery {

const Byte *Table::get_record(RecordOffset record_offset) const {
    
    if (record_offset.page_offset >= _header.page_count) {
        throw RecordOffsetOutOfRange{_name, record_offset};
    }
    auto cache_handle = PageManager::instance().load_page({_file_handle, record_offset.page_offset});
    auto cache = PageManager::instance().access_cache_for_reading(cache_handle);
    auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
    if (record_offset.slot_offset >= _header.slot_count_per_page) {
        throw RecordOffsetOutOfRange{_name, record_offset};
    }
    if (!data_page.header.slot_usage_bitmap[record_offset.slot_offset]) {
        throw RecordNotFound{_name, record_offset};
    }
    return &data_page.data[_header.record_length * record_offset.slot_offset];
    
}

RecordOffset Table::insert_record(const Byte *data) {
    
    if (_header.first_free_page == -1) {  // no available pages
        auto page_offset = static_cast<PageOffset>(_header.page_count);
        auto cache_handle = PageManager::instance().allocate_page({_file_handle, page_offset});
        auto cache = PageManager::instance().access_cache_for_writing(cache_handle);
        auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
        data_page.header.record_count = 1;
        data_page.header.slot_usage_bitmap.reset();
        data_page.header.slot_usage_bitmap[0] = true;
        data_page.header.next_free_page = -1;
        std::memmove(&data_page.data[0], data, _header.record_length);
        _header.first_free_page = page_offset;
        _header.page_count++;
        _header.record_count++;
        return {page_offset, 0};
    }
    
    // have available pages, take out the first free page.
    auto page_offset = _header.first_free_page;
    auto cache_handle = PageManager::instance().load_page({_file_handle, page_offset});
    auto cache = PageManager::instance().access_cache_for_writing(cache_handle);
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
                        &data_page.data[_header.record_length * slot], data, _header.record_length);
                    data_page.header.record_count++;
                    _header.record_count++;
                    if (data_page.header.record_count == _header.slot_count_per_page) {
                        _header.first_free_page = data_page.header.next_free_page;
                    }
                    return {page_offset, static_cast<SlotOffset>(slot)};
                }
            }
        }
    }
    
    // should have available slots but actually not found
    throw RecordSlotUsageBitmapCorrupt{_name, page_offset};
}

void Table::delete_record(RecordOffset record_offset) {
    
    if (record_offset.page_offset >= _header.page_count) {
        throw RecordOffsetOutOfRange{_name, record_offset};
    }
    auto cache_handle = PageManager::instance().load_page({_file_handle, record_offset.page_offset});
    auto cache = PageManager::instance().access_cache_for_writing(cache_handle);
    auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
    if (record_offset.slot_offset >= _header.slot_count_per_page) {
        throw RecordOffsetOutOfRange{_name, record_offset};
    }
    if (data_page.header.slot_usage_bitmap[record_offset.slot_offset]) {
        if (data_page.header.record_count == _header.slot_count_per_page) {  // a new free page.
            data_page.header.next_free_page = _header.first_free_page;
            _header.first_free_page = record_offset.page_offset;
        }
        data_page.header.slot_usage_bitmap[record_offset.slot_offset] = false;
        data_page.header.record_count--;
        _header.record_count--;
    } else {
        throw RecordNotFound{_name, record_offset};
    }
}

RecordOffset Table::next_record_offset(RecordOffset rid) const {
    auto init_slot = rid.slot_offset + 1;
    auto init_page = rid.page_offset;
    if (init_slot > _header.slot_count_per_page) {
        init_slot = 0;
        init_page++;
    }
    for (auto page = init_page; page < _header.page_count; page++) {
        auto cache_handle = PageManager::instance().load_page({_file_handle, page});
        auto cache = PageManager::instance().access_cache_for_reading(cache_handle);
        auto &p = MemoryMapper::map_memory<DataPage>(cache);
        for (auto slot = init_slot; slot < _header.slot_count_per_page; slot++) {
            if (p.header.slot_usage_bitmap[slot]) {
                return {page, slot};
            }
        }
        init_slot = 0;
    }
    return {-1, -1};
}

Table::~Table() {
    auto table_header_cache_handle = PageManager::instance().load_page({_file_handle, 0});
    auto table_header_cache = PageManager::instance().access_cache_for_writing(table_header_cache_handle);
    MemoryMapper::map_memory<TableHeader>(table_header_cache) = _header;
    PageManager::instance().close_file(_file_handle);
}

const std::string &Table::name() const noexcept {
    return _name;
}

const RecordDescriptor &Table::descriptor() const noexcept {
    return _header.record_descriptor;
}

void Table::add_foreign_key_reference(const std::string &referrer_table) noexcept {
    // just simply inc ref count
    _header.foreign_key_reference_count++;
}

void Table::drop_foreign_key_reference(const std::string &referrer_table) {
    if (_header.foreign_key_reference_count == 0) {
        throw NegativeForeignKeyReferenceCount{_name, referrer_table};
    }
    _header.foreign_key_reference_count--;
}

uint32_t Table::foreign_key_reference_count() const noexcept {
    return _header.foreign_key_reference_count;
}

RecordDescriptor &Table::descriptor() noexcept {
    return _header.record_descriptor;
}

Table::Table(std::string name, FileHandle fh, TableHeader th)
    : _name{std::move(name)}, _file_handle{fh}, _header{th} {
    for (auto i = 0; i < _header.record_descriptor.field_count; i++) {
        _column_offsets.emplace(th.record_descriptor.field_descriptors[i].name.data(), i);
    }
}

ColumnOffset Table::column_offset(const std::string &column_name) const {
    if (auto it = _column_offsets.find(column_name); it != _column_offsets.end()) {
        return it->second;
    }
    throw FieldNotFound{_name, column_name};
}

uint32_t Table::record_count() const noexcept {
    return _header.record_count;
}

bool Table::empty() const noexcept {
    return _header.record_count == 0;
}

RecordOffset Table::record_offset_begin() const {
    return next_record_offset({1, -1});
}

bool Table::is_record_offset_end(RecordOffset rid) const {
    return rid == RecordOffset{-1, -1};
}
    
}
