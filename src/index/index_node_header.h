//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_NODE_HEADER_H
#define WATERYSQL_INDEX_NODE_HEADER_H

#include <cstdint>
#include <bitset>

#include "../config/config.h"
#include "../record/record_offset.h"

namespace watery {

struct IndexNodeHeader {
    uint32_t key_count{0};
    bool is_leaf{false};
};

}

#endif  // WATERYSQL_INDEX_NODE_HEADER_H
