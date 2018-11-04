//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_NONCOPYABLE_H
#define WATERYSQL_NONCOPYABLE_H

namespace watery {

struct Noncopyable {
    
    Noncopyable() = default;
    Noncopyable(Noncopyable &&) = delete;
    Noncopyable(const Noncopyable &) = delete;
    Noncopyable &operator=(Noncopyable &&) = delete;
    Noncopyable &operator=(const Noncopyable &) = delete;
    
};

}

#endif  // WATERYSQL_NONCOPYABLE_H
