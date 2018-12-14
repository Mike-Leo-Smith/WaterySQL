//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_TYPE_TAG_H
#define WATERYSQL_TYPE_TAG_H

#include <cstdint>
#include <string>

namespace watery {

enum struct TypeTag : uint8_t {
    INTEGER = 0,
    FLOAT = 1,
    CHAR = 2,
    DATE = 3
};

template<typename OS>
OS &operator<<(OS &os, TypeTag tag) {
    switch (tag) {
        case TypeTag::INTEGER:
            os << "INT";
            break;
        case TypeTag::FLOAT:
            os << "FLOAT";
            break;
        case TypeTag::CHAR:
            os << "CHAR";
            break;
        case TypeTag::DATE:
            os << "DATE";
            break;
        default:
            os << "UNKNOWN";
            break;
    }
    return os;
}

}

#endif  // WATERYSQL_TYPE_TAG_H
