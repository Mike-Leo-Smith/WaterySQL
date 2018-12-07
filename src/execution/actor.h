//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_ACTOR_H
#define WATERYSQL_ACTOR_H

#include <functional>

namespace watery {

using Actor = std::function<void()>;

}

#endif  // WATERYSQL_ACTOR_H
