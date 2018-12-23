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
    
    bool has_nullable_fields() const noexcept {
        return std::any_of(
            field_descriptors.cbegin(),
            field_descriptors.cbegin() + field_count,
            [](auto d) { return d.constraints.nullable(); });
    }
    
    bool has_primary_key() const noexcept {
        return std::any_of(
            field_descriptors.cbegin(),
            field_descriptors.cbegin() + field_count,
            [](auto d) { return d.constraints.primary(); });
    }
    
    RecordDescriptor() = default;
    
    uint32_t length() const noexcept {
        auto size = 0u;
        for (auto i = 0; i < field_count; i++) {
            size += field_descriptors[i].data_descriptor.length();
        }
        if (has_nullable_fields()) {
            size += sizeof(NullFieldBitmap);
        }
        if (has_primary_key()) {
            size += sizeof(uint32_t);
        }
        return size;
    }
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_H
