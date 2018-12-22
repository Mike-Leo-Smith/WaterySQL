//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_RECORD_MANAGER_H
#define WATERYSQL_RECORD_MANAGER_H

#include <string>
#include <map>
#include <memory>
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
    std::map<std::string, Table> _open_tables;

protected:
    RecordManager() = default;

public:
    ~RecordManager();
    
    void create_table(const std::string &name, const RecordDescriptor &record_descriptor);
    Table &open_table(const std::string &name);
    bool is_table_open(const std::string &name) const;
    void close_table(const std::string &table);
    void close_all_tables();
    void delete_table(std::string name);
    
    const Byte *get_record(Table &table, RecordOffset record_offset);
    RecordOffset insert_record(Table &table, const Byte *data);
    void update_record(Table &table, RecordOffset record_offset, const Byte *data);
    void delete_record(Table &table, RecordOffset record_offset);
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
