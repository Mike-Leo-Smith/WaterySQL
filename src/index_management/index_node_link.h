//
// Created by Mike Smith on 2018/11/27.
//

#ifndef WATERYSQL_INDEX_NODE_LINK_H
#define WATERYSQL_INDEX_NODE_LINK_H

#include "../config/config.h"

namespace watery {

struct IndexNodeLink {
    
    PageOffset prev;
    PageOffset next;
    
};

}

#endif  // WATERYSQL_INDEX_NODE_LINK_H
