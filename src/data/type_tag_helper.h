//
// Created by Mike Smith on 2018-12-26.
//

#ifndef WATERYSQL_TYPE_TAG_HELPER_H
#define WATERYSQL_TYPE_TAG_HELPER_H

#include <string_view>

#include "../utility/type/non_constructible.h"
#include "type_tag.h"

namespace watery {

struct TypeTagHelper : NonConstructible {
    
    static constexpr std::string_view name(TypeTag tag) noexcept {
        switch (tag) {
            case TypeTag::INTEGER:
                return "INTEGER";
            case TypeTag::FLOAT:
                return "FLOAT";
            case TypeTag::CHAR:
                return "CHAR";
            case TypeTag::DATE:
                return "DATE";
            case TypeTag::BITMAP:
                return "BITMAP";
            default:
                return "UNKNOWN";
        }
    }
    
};

}

#endif  // WATERYSQL_TYPE_TAG_HELPER_H
