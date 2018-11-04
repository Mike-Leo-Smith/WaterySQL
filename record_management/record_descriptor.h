//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_RECORD_DESCRIPTOR_H
#define WATERYSQL_RECORD_DESCRIPTOR_H

#include <cstdint>
#include <array>
#include <numeric>

#include "../config/config.h"
#include "field_descriptor.h"

namespace watery {

struct RecordDescriptor final {
    
    uint32_t field_count;
    std::array<FieldDescriptor, MAX_FIELD_COUNT> fields;
    
    uint32_t length() const {
        return std::reduce(fields.begin(), fields.begin() + field_count, 0u,
            [](auto lhs, auto rhs) { return lhs + rhs.data_descriptor.size; });
    }
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_H
