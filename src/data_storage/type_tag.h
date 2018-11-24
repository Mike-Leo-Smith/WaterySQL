//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_TYPE_TAG_H
#define WATERYSQL_TYPE_TAG_H

#include <cstdint>

namespace watery {

enum struct TypeTag : uint16_t {
    INTEGER,
    FLOAT,
    CHAR,
    VARCHAR
};

}

#endif  // WATERYSQL_TYPE_TAG_H
