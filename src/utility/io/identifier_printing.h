//
// Created by Mike Smith on 2018-12-16.
//

#ifndef WATERYSQL_IDENTIFIER_PRINTING_H
#define WATERYSQL_IDENTIFIER_PRINTING_H

#include "../../config/config.h"

namespace watery {

template<typename Stream>
static inline Stream &operator<<(Stream &os, const Identifier &id) noexcept {
    os << id.data();
    return os;
}

}

#endif  // WATERYSQL_IDENTIFIER_PRINTING_H
