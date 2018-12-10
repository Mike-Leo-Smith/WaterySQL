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
    uint32_t field_count;
     bool null_mapped = false;
    
    RecordDescriptor(uint32_t fc, const std::array<FieldDescriptor, MAX_FIELD_COUNT> &fds)
        : field_count{fc}, field_descriptors{fds},
          null_mapped{std::any_of(fds.begin(), fds.begin() + fc, [](auto d) { return d.data_descriptor.nullable; })} {}
    
    RecordDescriptor(std::initializer_list<FieldDescriptor> fds)
        : field_count{static_cast<uint32_t>(fds.size())},
          null_mapped{std::any_of(fds.begin(), fds.begin() + fds.size(),
                                  [](auto d) { return d.data_descriptor.nullable; })} {
        std::copy(fds.begin(), fds.end(), field_descriptors.begin());
    }
    
    constexpr uint32_t length() const noexcept {
        auto size = 0u;
        for (auto i = 0; i < field_count; i++) {
            size += field_descriptors[i].record_field_length();
        }
        return null_mapped ? size + sizeof(FieldNullBitset) : size;
    }
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_H
