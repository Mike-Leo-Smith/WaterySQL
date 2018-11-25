//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_HEADER_H
#define WATERYSQL_INDEX_HEADER_H

#include <cstdint>
#include "../data_storage/data_descriptor.h"

namespace watery {

struct IndexHeader {
    
    DataDescriptor key_descriptor;
    uint32_t page_count;
    uint32_t key_length;
    uint32_t pointer_length;
    uint32_t child_count_per_node;
    
};

}

#endif  // WATERYSQL_INDEX_HEADER_H
