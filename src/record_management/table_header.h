//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_TABLE_HEADER_H
#define WATERYSQL_TABLE_HEADER_H

#include <cstdint>
#include "record_descriptor.h"

namespace watery {

struct TableHeader {
    RecordDescriptor record_descriptor;
    uint32_t page_count{1};
    uint32_t record_count{0};
    uint32_t record_length{0};
    uint32_t slot_count_per_page{0};
};

}

#endif  // WATERYSQL_TABLE_HEADER_H
