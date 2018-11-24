//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_NON_COPYABLE_H
#define WATERYSQL_NON_COPYABLE_H

namespace watery {

struct NonCopyable {
    
    NonCopyable() = default;
    NonCopyable(NonCopyable &&) = default;
    NonCopyable(const NonCopyable &) = delete;
    NonCopyable &operator=(NonCopyable &&) = default;
    NonCopyable &operator=(const NonCopyable &) = delete;
    
};

}

#endif  // WATERYSQL_NON_COPYABLE_H
