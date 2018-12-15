//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_STRING_VIEW_COPIER_H
#define WATERYSQL_STRING_VIEW_COPIER_H

#include <string_view>
#include "../type/non_trivial_constructible.h"

namespace watery {

struct StringViewCopier : NonTrivialConstructible {
    
    static inline void copy(std::string_view sv, char *buffer) noexcept {
        sv.copy(buffer, sv.size());
        buffer[sv.size()] = '\0';
    }
    
    template<typename Size, Size size>
    static inline void copy(std::string_view sv, std::array<Byte, size> &id) noexcept {
        copy(sv, id.data());
    }
    
};

}

#endif  // WATERYSQL_STRING_VIEW_COPIER_H
