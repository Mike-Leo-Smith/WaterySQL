//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_RECORD_MANAGER_H
#define WATERYSQL_RECORD_MANAGER_H

#include <string>
#include <optional>
#include "../filesystem_demo/fileio/FileManager.h"
#include "../filesystem_demo/bufmanager/BufPageManager.h"
#include "record.h"
#include "table_descriptor.h"
#include "table.h"

namespace watery {

class RecordManager {

private:
    FileManager _file_manager{};
    BufPageManager _page_manager{&_file_manager};
    
//    Record decode_record(const Table &table, )

public:
    void create_table(const std::string &name, const TableDescriptor &descriptor);
    std::optional<Table> open_table(const std::string &name);
    void close_table(int32_t id);
    void delete_table(const std::string &name);
    
    void insert_record(const Table &table, const Record &record);
    void update_record(const Table &table, const Record &record);
    void delete_record(const Table &table, int32_t slot);
    
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
