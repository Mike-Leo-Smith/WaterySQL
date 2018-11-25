//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_H
#define WATERYSQL_INDEX_H

#include <string>
#include "index_header.h"
#include "../config/config.h"

namespace watery {

struct Index {
    
    std::string name;
    FileHandle file_handle;
    IndexHeader header;

};

}

#endif  // WATERYSQL_INDEX_H
