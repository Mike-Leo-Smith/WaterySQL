//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_RECORD_OFFSET_H
#define WATERYSQL_RECORD_OFFSET_H

#include "../config/config.h"

namespace watery {

struct RecordOffset {
    
    PageOffset page_offset;
    SlotOffset slot_offset;
    
    constexpr bool operator==(const RecordOffset &rhs) noexcept {
        return page_offset == rhs.page_offset && slot_offset == rhs.slot_offset;
    }
    
    constexpr bool operator!=(const RecordOffset &rhs) noexcept {
        return page_offset != rhs.page_offset || slot_offset != rhs.slot_offset;
    }
    
    std::string to_string() const {
        return std::string{"("}
            .append(std::to_string(page_offset)).append(", ")
            .append(std::to_string(slot_offset)).append(")");
    }
    
};

}

#endif  // WATERYSQL_RECORD_OFFSET_H
