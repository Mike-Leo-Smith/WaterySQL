#include <utility>

//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_H
#define WATERYSQL_INDEX_H

#include <string>
#include "index_header.h"
#include "../config/config.h"
#include "../data_storage/index_key_comparator.h"

namespace watery {

struct Index {
    
    std::string name{};
    FileHandle file_handle{};
    IndexHeader header{};
    IndexKeyComparator comparator{};
    
    Index() = default;
    
    Index(std::string name, FileHandle fh, IndexHeader h)
        : name{std::move(name)}, file_handle{fh}, header{h}, comparator{h.key_descriptor, h.unique} {}
    
    Index(Index &&) = default;
    Index(const Index &) = default;
    Index &operator=(Index &&) = default;
    Index &operator=(const Index &) = default;
};

}

#endif  // WATERYSQL_INDEX_H
