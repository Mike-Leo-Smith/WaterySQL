//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_TABLE_H
#define WATERYSQL_TABLE_H

#include <string>

#include "table_header.h"
#include "../config/config.h"
#include "../data/record_descriptor.h"
#include "../page/page_manager.h"
#include "../error/record_offset_out_of_range.h"
#include "data_page.h"
#include "../utility/memory/memory_mapper.h"
#include "../error/record_not_found.h"

namespace watery {

class Table {

private:
    std::string _name;
    FileHandle _file_handle{-1};
    TableHeader _header;
    std::map<std::string, ColumnOffset> _column_offsets;

public:
    Table(std::string name, FileHandle fh, TableHeader th);
    ~Table();
    
    const std::string &name() const noexcept;
    RecordDescriptor &descriptor() noexcept;
    const RecordDescriptor &descriptor() const noexcept;
    ColumnOffset column_offset(const std::string &column_name) const;
    uint32_t record_count() const noexcept;
    bool empty() const noexcept;
    
    const Byte *get_record(RecordOffset record_offset) const;
    RecordOffset insert_record(const Byte *data);
    void delete_record(RecordOffset record_offset);
    
    uint32_t record_reference_count(RecordOffset rid) const;
    void add_record_reference_count(RecordOffset rid);
    void drop_record_reference_count(RecordOffset rid);
    
    template<typename Func>
    void update_record(RecordOffset record_offset, Func &&update) {
        if (record_offset.page_offset >= _header.page_count) {
            throw RecordOffsetOutOfRange{_name, record_offset};
        }
        auto cache_handle = PageManager::instance().load_page({_file_handle, record_offset.page_offset});
        auto cache = PageManager::instance().access_cache_for_writing(cache_handle);
        auto &data_page = MemoryMapper::map_memory<DataPage>(cache);
        if (record_offset.slot_offset >= _header.slot_count_per_page) {
            throw RecordOffsetOutOfRange{_name, record_offset};
        }
        if (!data_page.header.slot_usage_bitmap[record_offset.slot_offset]) {
            throw RecordNotFound{_name, record_offset};
        }
        update(&data_page.data[_header.record_length * record_offset.slot_offset]);
    }
    
    RecordOffset record_offset_begin() const;
    RecordOffset next_record_offset(RecordOffset rid) const;
    bool is_record_offset_end(RecordOffset rid) const;
    
    void add_foreign_key_reference(const std::string &referrer_table) noexcept;
    void drop_foreign_key_reference(const std::string &referrer_table);
    uint32_t foreign_key_reference_count() const noexcept;
    
};
    
}

#endif  // WATERYSQL_TABLE_H
