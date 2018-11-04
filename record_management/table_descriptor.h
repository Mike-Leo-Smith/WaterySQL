//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_TABLE_DESCRIPTOR_H
#define WATERYSQL_TABLE_DESCRIPTOR_H

#include <bitset>
#include "record_descriptor.h"

namespace watery {

struct TableDescriptor final {
    
    uint32_t page_count;
    uint32_t record_count;
    
    RecordDescriptor record_descriptor;
    
};

}

#endif  // WATERYSQL_TABLE_DESCRIPTOR_H
