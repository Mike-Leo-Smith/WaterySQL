//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_RECORD_MANAGER_H
#define WATERYSQL_RECORD_MANAGER_H

#include <string>
#include "../filesystem_demo/fileio/FileManager.h"
#include "../filesystem_demo/bufmanager/BufPageManager.h"
#include "record.h"

namespace watery {

class RecordManager {

private:
    FileManager *_file_manager;
    BufPageManager *_page_manager;

public:
    void create_table(const std::string &name);
    int32_t open_table(const std::string &name);
    int32_t close_table(const std::string &name);
    void delete_table(const std::string &name);
    
    void insert_record(int32_t file_id, Record );
    
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
