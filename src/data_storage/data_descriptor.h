//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_DATA_DESCRIPTOR_H
#define WATERYSQL_DATA_DESCRIPTOR_H

#include "type_tag.h"

namespace watery {

struct DataDescriptor {
    
    TypeTag type;
    uint16_t size_hint;
    
    constexpr uint32_t length() const noexcept {
        switch (type) {
        case TypeTag::INTEGER:
            return sizeof(int32_t);
        case TypeTag::FLOAT:
            return sizeof(float);
        case TypeTag::VARCHAR:
            return size_hint;
        default:
            return 0;
        }
    }
    
    constexpr bool operator==(DataDescriptor rhs) const noexcept {
        return type == rhs.type && size_hint == rhs.size_hint;
    }
    
    constexpr bool operator!=(DataDescriptor rhs) const noexcept {
        return !(*this == rhs);
    }
};

}

#endif  // WATERYSQL_DATA_DESCRIPTOR_H
