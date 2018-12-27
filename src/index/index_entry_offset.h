//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_ENTRY_OFFSET_H
#define WATERYSQL_INDEX_ENTRY_OFFSET_H

#include <string>

#include "../config/config.h"

namespace watery {

struct IndexEntryOffset {
    
    PageOffset page_offset;
    ChildOffset child_offset;
    
    IndexEntryOffset(PageOffset p, ChildOffset c)
        : page_offset{p}, child_offset{c} {}
    
    std::string to_string() const noexcept {
        return std::string{"("}
            .append(std::to_string(page_offset)).append(", ")
            .append(std::to_string(child_offset)).append(")");
    }
    
};

}

#endif  // WATERYSQL_INDEX_ENTRY_OFFSET_H
