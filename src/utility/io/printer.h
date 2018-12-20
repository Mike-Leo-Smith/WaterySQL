//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_PRINTER_H
#define WATERYSQL_PRINTER_H

#include <iostream>
#include "../type/non_trivial_constructible.h"

namespace watery {

struct Printer : NonTrivialConstructible {
    
    template<typename OStream, typename ...Args>
    static void println(OStream &&os, Args &&...args) noexcept {
        print(std::forward<OStream>(os), std::forward<Args>(args)...);
        os << std::endl;
    }
    
    template<typename OStream, typename ...Args>
    static void print(OStream &os, Args &&...args) noexcept {
        auto &&_ = (os << ... << std::forward<Args>(args));  // making compilers happy when args is empty.
    }
    
};

}

#endif  // WATERYSQL_PRINTER_H
