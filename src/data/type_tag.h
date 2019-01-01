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
    DATE = 3,
    BITMAP = 4
};

}

#endif  // WATERYSQL_TYPE_TAG_H
