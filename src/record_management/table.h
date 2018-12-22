//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_TABLE_H
#define WATERYSQL_TABLE_H

#include <string>
#include "../config/config.h"
#include "../data_storage/record_descriptor.h"
#include "table_header.h"

namespace watery {

struct Table {
    std::string name;
    FileHandle file_handle{-1};
    TableHeader header;
    
    Table(std::string name, FileHandle fh, TableHeader th)
        : name{std::move(name)}, file_handle{fh}, header{th} {}
    
    Table() = default;
};

}

#endif  // WATERYSQL_TABLE_H
