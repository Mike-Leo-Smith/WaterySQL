//
// Created by Mike Smith on 2018/11/27.
//

#ifndef WATERYSQL_INDEXNODELINK_H
#define WATERYSQL_INDEXNODELINK_H

#include "../config/config.h"

namespace watery {

struct IndexNodeLink {
    
    PageOffset prev;
    PageOffset next;
    
};

}

#endif  // WATERYSQL_INDEXNODELINK_H
