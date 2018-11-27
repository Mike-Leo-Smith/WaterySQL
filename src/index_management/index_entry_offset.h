//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_ENTRY_OFFSET_H
#define WATERYSQL_INDEX_ENTRY_OFFSET_H

#include "../config/config.h"

namespace watery {

struct IndexEntryOffset {
    
    PageOffset page_offset;
    ChildOffset child_offset;
    
    IndexEntryOffset(PageOffset p, ChildOffset c)
        : page_offset{p}, child_offset{c} {}
    
};

}

#endif  // WATERYSQL_INDEX_ENTRY_OFFSET_H
