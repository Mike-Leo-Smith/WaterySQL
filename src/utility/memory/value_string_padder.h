//
// Created by Mike Smith on 2018-12-31.
//

#ifndef WATERYSQL_VALUE_STRING_PADDER_H
#define WATERYSQL_VALUE_STRING_PADDER_H

#include <string>
#include <vector>
#include "../type/non_constructible.h"
#include "../../data/data_descriptor.h"

namespace watery {

struct ValueStringPadder : NonConstructible {
    
    static std::string pad(DataDescriptor data_desc, std::string field_str) noexcept {
        static const std::string padding_str{"          "};
        switch (data_desc.type) {
            case TypeTag::INTEGER:
            case TypeTag::FLOAT:
                return field_str.size() < padding_str.size() ?
                       (padding_str.c_str() + field_str.size()) + field_str :
                       field_str;
            case TypeTag::CHAR:
                field_str.resize(data_desc.length, ' ');
                return field_str;
            default:
                return field_str;
        }
    }
    
};

}

#endif  // WATERYSQL_VALUE_STRING_PADDER_H
