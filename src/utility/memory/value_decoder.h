//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_VALUE_DECODER_H
#define WATERYSQL_VALUE_DECODER_H

#include <cmath>
#include <cstdint>
#include <string_view>
#include <charconv>

#include "../type/non_trivial_constructible.h"
#include "../../error/value_decoder_error.h"
#include "../time/date_validator.h"
#include "../../data/type_tag_helper.h"

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
    
    static int32_t decode_date(std::string_view raw) {  // string look like '1998-08-10'
        if (raw.size() != 12 || raw[0] != '\'' || raw[5] != '-' || raw[8] != '-' || raw[11] != '\'') {
            throw ValueDecoderError{raw, "DATE", "of unrecognized pattern."};
        }
        auto year = decode_integer(raw.substr(1, 4));
        auto month = decode_integer(raw.substr(6, 2));
        auto day = decode_integer(raw.substr(9, 2));
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
    
    static std::string_view decode_char(std::string_view raw) {
        if (raw.size() < 2 || raw[0] != '\'' || raw.back() != '\'') {
            throw ValueDecoderError{raw, "CHAR", "it cannot be seen as a string."};
        }
        return raw.substr(1, raw.size() - 2);  // remove quotation marks.
    }
    
    static void decode(TypeTag type, std::string_view raw, Byte *result, uint32_t max_size = 0) {
        switch (type) {
            case TypeTag::INTEGER:
                MemoryMapper::map_memory<int32_t>(result) = ValueDecoder::decode_integer(raw);
                break;
            case TypeTag::FLOAT:
                MemoryMapper::map_memory<float>(result) = ValueDecoder::decode_float(raw);
                break;
            case TypeTag::CHAR:
                if (max_size > 0 && raw.size() - 2 > max_size) {
                    throw ValueDecoderError{raw, "CHAR", "it is too long."};
                }
                StringViewCopier::copy(ValueDecoder::decode_char(raw), result);
                break;
            case TypeTag::DATE:
                MemoryMapper::map_memory<int32_t>(result) = ValueDecoder::decode_date(raw);
                break;
            default:
                throw ValueDecoderError{raw, TypeTagHelper::name(type), "of unknown type tag."};
        }
    }
    
};

}

#endif  // WATERYSQL_VALUE_DECODER_H
