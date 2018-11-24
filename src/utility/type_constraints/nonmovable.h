//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_NONMOVABLE_H
#define WATERYSQL_NONMOVABLE_H

namespace watery {

struct Nonmovable {
    Nonmovable() = default;
    Nonmovable(Nonmovable &&) = delete;
    Nonmovable &operator=(Nonmovable &&) = delete;
};

}

#endif  // WATERYSQL_NONMOVABLE_H
