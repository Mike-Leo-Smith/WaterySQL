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
#include "../error/data_error.h"

namespace watery {

struct RecordDescriptor final {
    
    std::array<FieldDescriptor, MAX_FIELD_COUNT> field_descriptors{};
    uint32_t field_count{0};
    uint32_t length{0};
    std::array<uint32_t, MAX_FIELD_COUNT> field_offsets{};
    ColumnOffset primary_key_column_offset{-1};
    bool null_mapped{false};
    
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
    
    constexpr bool reference_counted() const {
        return primary_key_column_offset != -1;
    }
    
    ColumnOffset get_column_offset(std::string_view col_name) const {
        for (auto i = 0; i < field_count; i++) {
            if (col_name == field_descriptors[i].name.data()) {
                return i;
            }
        }
        throw DataError{std::string{"Failed to get column offset of \""}.append(col_name).append("\".")};
    }
    
    RecordDescriptor() = default;
    
};

}

#endif  // WATERYSQL_RECORD_DESCRIPTOR_H
