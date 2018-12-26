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
};

static constexpr bool operator==(const RecordOffset &lhs, const RecordOffset &rhs) noexcept {
    return lhs.page_offset == rhs.page_offset && lhs.slot_offset == rhs.slot_offset;
}

static constexpr bool operator!=(const RecordOffset &lhs, const RecordOffset &rhs) noexcept {
    return lhs.page_offset != rhs.page_offset || lhs.slot_offset != rhs.slot_offset;
}

}

#endif  // WATERYSQL_RECORD_OFFSET_H
