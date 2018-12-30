//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_NON_CONSTRUCTIBLE_H
#define WATERYSQL_NON_CONSTRUCTIBLE_H

namespace watery {

struct NonConstructible {
    NonConstructible() = delete;
};

}

#endif  // WATERYSQL_NON_CONSTRUCTIBLE_H
