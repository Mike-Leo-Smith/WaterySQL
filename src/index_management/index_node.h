//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_NODE_H
#define WATERYSQL_INDEX_NODE_H

#include <array>
#include <bitset>
#include "../record_management/record_offset.h"
#include "index_node_header.h"

namespace watery {

struct IndexNode {
    IndexNodeHeader header;
    alignas(8) Byte fields[];  // serves as position indicator.
};

}

#endif  // WATERYSQL_INDEX_NODE_H
