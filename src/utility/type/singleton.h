//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_SINGLETON_H
#define WATERYSQL_SINGLETON_H

#include "non_copyable.h"
#include "non_movable.h"

namespace watery {

template<typename UnderlyingClass>
class Singleton : NonCopyable, NonMovable {

protected:
    Singleton() = default;

private:
    struct UnderlyingClassConstructor : UnderlyingClass {
        friend class Singleton;
        UnderlyingClassConstructor() : UnderlyingClass{} {}
    };

public:
    static UnderlyingClass &instance() {
        static UnderlyingClassConstructor singleton;
        return singleton;
    }
    
};

}

#endif  // WATERYSQL_SINGLETON_H
