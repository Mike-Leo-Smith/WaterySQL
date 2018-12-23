//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_RECORD_DESCRIPTOR_H
#define WATERYSQL_RECORD_DESCRIPTOR_H

#include <cstdint>
#include <array>
#include <numeric>
#include <algorithm>

#include "../config/config.h"
#include "field_descriptor.h"

namespace watery {

struct RecordDescriptor final {
    
    std::array<FieldDescriptor, MAX_FIELD_COUNT> field_descriptors{};
    uint32_t field_count{0};
    uint16_t length{0};
    bool null_mapped{false};
    bool reference_counted{false};
    
    RecordDescriptor(uint32_t fc, const std::array<FieldDescriptor, MAX_FIELD_COUNT> &fds)
        : field_count{fc},
          field_descriptors{fds} {}
    
    template<typename Container>
    explicit RecordDescriptor(const Container &fds)
        : field_count{static_cast<uint32_t>(fds.size())} {
        std::copy(fds.cbegin(), fds.cend(), field_descriptors.begin());
    }
    
    RecordDescriptor(std::initializer_list<FieldDescriptor> fds)
        : field_count{static_cast<uint32_t>(fds.size())} {
        std::copy(fds.begin(), fds.end(), field_descriptors.begin());
    }
    
    RecordDescriptor() = default;
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_H
