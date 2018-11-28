//
// Created by Mike Smith on 2018/11/28.
//

#ifndef WATERYSQL_SGN_H
#define WATERYSQL_SGN_H

#include <cstdint>

namespace watery {

template<typename T>
int32_t sgn(T &&x) noexcept {
    return x < 0 ? -1 : (x > 0 ? 1 : 0);
}

}

#endif  // WATERYSQL_SGN_H
