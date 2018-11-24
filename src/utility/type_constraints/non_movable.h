//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_NON_MOVABLE_H
#define WATERYSQL_NON_MOVABLE_H

namespace watery {

struct NonMovable {
    NonMovable() = default;
    NonMovable(NonMovable &&) = delete;
    NonMovable &operator=(NonMovable &&) = delete;
};

}

#endif  // WATERYSQL_NON_MOVABLE_H
