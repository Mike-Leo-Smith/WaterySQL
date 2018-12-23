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
    void update_record(std::weak_ptr<Table> table, RecordOffset record_offset, const Byte *data);
    void delete_record(std::weak_ptr<Table> table, RecordOffset record_offset);
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
