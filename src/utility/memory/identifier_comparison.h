//
// Created by Mike Smith on 2018-12-16.
//

#ifndef WATERYSQL_IDENTIFIER_COMPARISON_H
#define WATERYSQL_IDENTIFIER_COMPARISON_H

#include <string_view>
#include "../../config/config.h"

namespace watery {

static inline bool operator==(std::string_view lhs, const Identifier &rhs) {
    return lhs == rhs.data();
}

static inline bool operator==(const Identifier &lhs, std::string_view rhs) {
    return lhs.data() == rhs;
}

}

#endif  // WATERYSQL_IDENTIFIER_COMPARISON_H
