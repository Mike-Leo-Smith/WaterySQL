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
    bool null_mapped = false;
    
    RecordDescriptor(uint32_t fc, const std::array<FieldDescriptor, MAX_FIELD_COUNT> &fds)
        : field_count{fc}, field_descriptors{fds}, null_mapped{has_nullable_fields(fds.begin(), fds.begin() + fc)} {}
    
    template<typename Container>
    explicit RecordDescriptor(const Container &fds)
        : field_count{static_cast<uint32_t>(fds.size())}, null_mapped{has_nullable_fields(fds.begin(), fds.end())} {
        std::copy(fds.begin(), fds.end(), field_descriptors.begin());
    }
    
    RecordDescriptor(std::initializer_list<FieldDescriptor> fds)
        : field_count{static_cast<uint32_t>(fds.size())}, null_mapped{has_nullable_fields(fds.begin(), fds.end())} {
        std::copy(fds.begin(), fds.end(), field_descriptors.begin());
    }
    
    template<typename Iter>
    static bool has_nullable_fields(const Iter &begin, const Iter &end) {
        return std::any_of(begin, end, [](auto d) { return d.constraints.nullable(); });
    }
    
    RecordDescriptor() = default;
    
    constexpr uint32_t length() const noexcept {
        auto size = 0u;
        for (auto i = 0; i < field_count; i++) {
            size += field_descriptors[i].data_descriptor.length();
        }
        return null_mapped ? size + sizeof(NullFieldBitmap) : size;
    }
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_H
