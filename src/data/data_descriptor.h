//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_DATA_DESCRIPTOR_H
#define WATERYSQL_DATA_DESCRIPTOR_H

#include "type_tag.h"

namespace watery {

struct DataDescriptor {
    
    TypeTag type{};
    uint16_t length{};
    
    DataDescriptor() = default;
    
    DataDescriptor(TypeTag t, uint16_t sh)
        : type{t} {
        switch (type) {
            case TypeTag::INTEGER:
            case TypeTag::FLOAT:
            case TypeTag::DATE:
                length = 4;
                break;
            case TypeTag::CHAR:
                length = sh + uint16_t{1};  // strings are zero-ended
                break;
            default:
                length = 0;
        }
    }
    
    bool operator==(DataDescriptor rhs) const noexcept {
        return type == rhs.type && length == rhs.length;
    }
    
    bool operator!=(DataDescriptor rhs) const noexcept {
        return !(*this == rhs);
    }
};

}

#endif  // WATERYSQL_DATA_DESCRIPTOR_H
