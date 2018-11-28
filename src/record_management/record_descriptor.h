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
    std::array<FieldDescriptor, MAX_FIELD_COUNT> field_descriptors{};
    
    RecordDescriptor(uint32_t fc, const std::array<FieldDescriptor, MAX_FIELD_COUNT> &fds)
        : field_count{fc}, field_descriptors{fds} {}
    
    RecordDescriptor(std::initializer_list<FieldDescriptor> fds)
        : field_count{static_cast<uint32_t>(fds.size())} {
        std::copy(fds.begin(), fds.end(), field_descriptors.begin());
    }
    
    uint32_t calculate_length() const {
        auto size = 0u;
        for (auto i = 0; i < field_count; i++) {
            size += field_descriptors[i].data_descriptor.size;
        }
        return size;
//        return sizeof(uint32_t) +
//               std::reduce(field_descriptors.begin(), field_descriptors.begin() + field_count, 0u,
//                           [](auto lhs, auto rhs) { return lhs + rhs.data_descriptor.size; });
    }
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_H
