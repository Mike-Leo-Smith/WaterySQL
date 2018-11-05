//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_TABLE_DESCRIPTOR_H
#define WATERYSQL_TABLE_DESCRIPTOR_H

#include <bitset>
#include "record_descriptor.h"

namespace watery {

struct TableDescriptor final {
    
    RecordDescriptor record_descriptor;
    
    uint32_t page_count;
    uint32_t record_count;
    
    explicit TableDescriptor(const RecordDescriptor &rd, uint32_t page_count = 0, uint32_t record_count = 0)
        : record_descriptor{rd},  page_count{page_count}, record_count{record_count} {}
    
};

}

#endif  // WATERYSQL_TABLE_DESCRIPTOR_H
