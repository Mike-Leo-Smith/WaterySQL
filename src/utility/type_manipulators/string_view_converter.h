//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_STRING_VIEW_CONVERTER_H
#define WATERYSQL_STRING_VIEW_CONVERTER_H

#include <cstdint>
#include <string_view>

#include "../type_constraints/non_trivial_constructible.h"

namespace watery {

namespace _impl {

template<typename T>
struct StringViewConverterImpl {
    constexpr static T convert(std::string_view sv) {
        return T{};
    }
};

template<>
struct StringViewConverterImpl<int32_t> {

};

}

struct StringViewConverter : NonTrivialConstructible {



};

}

#endif  // WATERYSQL_STRING_VIEW_CONVERTER_H
