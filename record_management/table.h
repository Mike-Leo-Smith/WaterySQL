#include <utility>

//
// Created by Mike Smith on 2018/11/5.
//

#ifndef WATERYSQL_TABLE_H
#define WATERYSQL_TABLE_H

#include <string>
#include "../utility/noncopyable.h"
#include "table_descriptor.h"

namespace watery {

struct Table {
    std::string name;
    TableDescriptor descriptor;
    int32_t file_id;
    
    Table(std::string name, const TableDescriptor &descriptor, int32_t file_id = -1)
        : name{std::move(name)}, file_id{file_id}, descriptor{descriptor} {}
};

}

#endif  // WATERYSQL_TABLE_H
