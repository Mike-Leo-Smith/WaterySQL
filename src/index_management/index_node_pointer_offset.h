//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_NODE_POINTER_OFFSET_H
#define WATERYSQL_INDEX_NODE_POINTER_OFFSET_H

#include "../config/config.h"

namespace watery {

struct IndexNodePointerOffset {
    
    PageOffset page_offset;
    ChildOffset child_offset;
    
};

}

#endif  // WATERYSQL_INDEX_NODE_POINTER_OFFSET_H
