//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_HEADER_H
#define WATERYSQL_INDEX_HEADER_H

#include <cstdint>
#include "../config/config.h"
#include "../data/field_descriptor.h"

namespace watery {

struct IndexHeader {
    
    DataDescriptor key_descriptor{};
    bool unique{false};
    uint32_t key_length{0};
    uint32_t data_length{0};
    uint32_t page_count{1};
    uint32_t key_count_per_node{0};
    PageOffset root_offset{-1};
    
};

}

#endif  // WATERYSQL_INDEX_HEADER_H
