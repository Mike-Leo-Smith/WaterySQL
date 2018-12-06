//
// Created by Mike Smith on 2018-12-06.
//

#ifndef WATERYSQL_TOKEN_OFFSET_H
#define WATERYSQL_TOKEN_OFFSET_H

#include <cstdint>

namespace watery {

struct TokenOffset {
    int32_t line{0};
    int32_t column{0};
};

}

#endif  // WATERYSQL_TOKEN_OFFSET_H
