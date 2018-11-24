//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_ERROR_PRINTER_H
#define WATERYSQL_ERROR_PRINTER_H

#include <iostream>
#include "printer.h"

namespace watery {

namespace _impl {

template<typename OStream>
void _print_error_impl(
    OStream &os, std::string_view file, std::string_view function,
    size_t line, const std::exception &e) {
    Printer::print(std::cerr, "Error caught in ",
                   "file \"", file, "\", ",
                   "function \"", function, "\", ",
                   "line #", line, ", ",
                   "becasue of ", (e).what());
}

}

#define print_error(os, e) _impl::_print_error_impl(os, __FILE__, __FUNCTION__, __LINE__, e)
    
}

#endif  // WATERYSQL_ERROR_PRINTER_H
