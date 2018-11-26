//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_NODE_HEADER_H
#define WATERYSQL_INDEX_NODE_HEADER_H

#include <cstdint>
#include <bitset>

#include "../config/config.h"
#include "../record_management/record_offset.h"

namespace watery {

struct IndexNodeHeader {
    uint32_t child_count;
    PageOffset parent_page_offset;
    bool is_leaf;
};

}

#endif  // WATERYSQL_INDEX_NODE_HEADER_H
