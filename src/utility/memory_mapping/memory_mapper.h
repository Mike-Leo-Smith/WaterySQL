//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_MEMORY_MAPPER_H
#define WATERYSQL_MEMORY_MAPPER_H

#include <type_traits>

#include "../type_constraints/non_trivial_constructible.h"
#include "../../config/config.h"

namespace watery {

struct MemoryMapper : NonTrivialConstructible {
    
    template<typename T>
    static inline T &map_memory(Byte *buffer) noexcept {
        return (*reinterpret_cast<T *>(buffer));
    }
    
    template<typename T>
    static inline const T &map_memory(const Byte *buffer) noexcept {
        return (*reinterpret_cast<const T *>(buffer));
    }
    
};

}

#endif  // WATERYSQL_MEMORY_MAPPER_H
