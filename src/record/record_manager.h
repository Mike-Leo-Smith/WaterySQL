//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_RECORD_MANAGER_H
#define WATERYSQL_RECORD_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <array>
#include "../page/page_manager.h"
#include "../data/record_descriptor.h"
#include "table.h"
#include "record_offset.h"
#include "data_page.h"
#include "../error/record_manager_error.h"
#include "../utility/memory/memory_mapper.h"

namespace watery {

class RecordManager : public Singleton<RecordManager> {

private:
    PageManager &_page_manager = PageManager::instance();
    std::map<std::string, std::shared_ptr<Table>> _open_tables;

protected:
    RecordManager() = default;
    
    static std::shared_ptr<Table> _try_lock_table_weak_pointer(std::weak_ptr<Table> table);
    void _close_table(const std::shared_ptr<Table> &table) const;

public:
    ~RecordManager();
    
    void create_table(const std::string &name, const RecordDescriptor &record_descriptor);
    std::weak_ptr<Table> open_table(const std::string &name);
    bool is_table_open(const std::string &name) const;
    void close_table(const std::string &name);
    void close_all_tables();
    void delete_table(std::string name);
    
    const Byte *get_record(std::weak_ptr<Table> table, RecordOffset record_offset);
    RecordOffset insert_record(std::weak_ptr<Table> table, const Byte *data);
    void overwrite_record(std::weak_ptr<Table> t, RecordOffset record_offset, const Byte *data);
    void delete_record(std::weak_ptr<Table> table, RecordOffset record_offset);
    
    RecordOffset next_record_offset(std::weak_ptr<Table> table, RecordOffset rid);
    
    template<typename Func>
    void update_record(std::weak_ptr<Table> t, RecordOffset record_offset, Func &&update) {
        auto table = _try_lock_table_weak_pointer(std::move(t));
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
        update(&data_page.data[table->header.record_length * record_offset.slot_offset]);
    }
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
