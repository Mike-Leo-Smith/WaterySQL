//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_VALUE_DECODER_H
#define WATERYSQL_VALUE_DECODER_H

#include <cstdint>
#include <string_view>
#include <charconv>

#include "../type/non_trivial_constructible.h"
#include "../../errors/value_decoder_error.h"
#include "../time/date_validator.h"

namespace watery {

struct ValueDecoder : NonTrivialConstructible {
    
    static int32_t decode_integer(std::string_view raw) {
        int32_t result;
        auto ec = std::from_chars(raw.begin(), raw.end(), result).ec;
        if (ec == std::errc::invalid_argument) {
            throw ValueDecoderError{raw, "INT", "of unrecognized pattern."};
        }
        if (ec == std::errc::result_out_of_range) {
            throw ValueDecoderError{raw, "INT", "the result is out of range."};
        }
        return result;
    }
    
    static int32_t decode_date(std::string_view raw) {  // some string look like "1998-08-10"
        if (raw[4] != '-' || raw[7] != '-') {
            throw ValueDecoderError{raw, "DATE", "of unrecognized pattern."};
        }
        auto year = decode_integer(raw.substr(0, 4));
        auto month = decode_integer(raw.substr(5, 2));
        auto day = decode_integer(raw.substr(8, 2));
        if (!DateValidator::validate(year, month, day)) {
            throw ValueDecoderError{raw, "DATE", "the date is not in a valid range."};
        }
        return (year << 16) | (month << 8) | day;
    }
    
    static float decode_float(std::string_view raw) {
        char *end;
        auto result = std::strtof(raw.begin(), &end);
        if (result == HUGE_VALF) {
            throw ValueDecoderError{raw, "FLOAT", "of overflow."};
        }
        if (end == raw.begin()) {
            throw ValueDecoderError{raw, "FLOAT", "of the mismatched pattern."};
        }
        return result;
    }
    
};

}

#endif  // WATERYSQL_VALUE_DECODER_H
