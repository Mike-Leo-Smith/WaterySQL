//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_PRINTER_H
#define WATERYSQL_PRINTER_H

#include <iostream>
#include "../type_constraints/non_trivial_constructible.h"

namespace watery {

struct Printer : NonTrivialConstructible {
    template<typename OStream, typename ...Args>
    static void print(OStream &os, Args &&...args) {
        (os << ... << args) << std::endl;
    }
};

}

#endif  // WATERYSQL_PRINTER_H
