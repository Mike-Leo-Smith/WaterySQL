//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_DATE_VALIDATOR_H
#define WATERYSQL_DATE_VALIDATOR_H

#include <cstdint>
#include "../type/non_trivial_constructible.h"

namespace watery {

struct DateValidator : NonTrivialConstructible {
    
    static bool validate(int32_t year, int32_t month, int32_t day) noexcept {
        static constexpr int days_in_month[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        return year >= 0 && year <= 9999 && month > 0 && month <= 12 && day > 0 &&
               day <= (month == 2 && ((year % 100 != 0 && year % 4 == 0) || year % 400 == 0)) + days_in_month[month];
    }
    
};

}

#endif  // WATERYSQL_DATE_VALIDATOR_H
