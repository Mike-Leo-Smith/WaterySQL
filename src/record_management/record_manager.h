//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_RECORD_MANAGER_H
#define WATERYSQL_RECORD_MANAGER_H

#include <string>
#include <unordered_map>
#include <array>
#include "../page_management/page_manager.h"
#include "../data_storage/record_descriptor.h"
#include "table.h"
#include "record_offset.h"
#include "data_page.h"
#include "../errors/record_manager_error.h"
#include "../utility/memory/memory_mapper.h"

namespace watery {

class RecordManager : public Singleton<RecordManager> {

private:
    PageManager &_page_manager = PageManager::instance();
    std::array<std::unordered_map<BufferHandle, BufferOffset>, MAX_FILE_COUNT> _used_buffers;
    std::unordered_map<std::string, Table> _open_tables;

protected:
    RecordManager() = default;

protected:
    template<typename Visitor>
    decltype(auto) _visit_record(Table &table, RecordOffset record_offset, Visitor &&visitor) {
        if (record_offset.page_offset >= table.header.page_count) {
            throw RecordManagerError{
                "Failed to get record whose expected page offset is greater than table page count."};
        }
        auto page_handle = _page_manager.get_page(table.file_handle, record_offset.page_offset);
        _page_manager.mark_page_accessed(page_handle);
        _used_buffers[table.file_handle].emplace(page_handle.buffer_handle, page_handle.buffer_offset);
        
        auto &data_page = MemoryMapper::map_memory<DataPage>(page_handle.data);
        if (record_offset.slot_offset >= table.header.slot_count_per_page) {
            throw RecordManagerError{
                "Failed to get record whose expected slot offset is greater than slot count in page."};
        }
        return visitor(table, page_handle, data_page, record_offset);
    }
    
    void _close_table(const Table &table) noexcept;

public:
    ~RecordManager();
    
    void create_table(const std::string &name, const RecordDescriptor &record_descriptor);
    Table &open_table(const std::string &name);
    bool is_table_open(const std::string &name) const;
    void close_table(const std::string &table);
    void close_all_tables();
    void delete_table(const std::string &name);
    
    const Byte *get_record(Table &table, RecordOffset record_offset);
    RecordOffset insert_record(Table &table, const Byte *data);
    void update_record(Table &table, RecordOffset record_offset, const Byte *data);
    void delete_record(Table &table, RecordOffset record_offset);
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
