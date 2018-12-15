//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_VALUE_DECODER_ERROR_H
#define WATERYSQL_VALUE_DECODER_ERROR_H

#include "error.h"

namespace watery {

struct ValueDecoderError : public Error {
    explicit ValueDecoderError(std::string_view raw, std::string_view type, std::string_view reason) noexcept
        : Error{"ValueDecoderError",
                std::string{"Failed to decode \""}
                    .append(raw).append("\" into type ").append(type)
                    .append(" because ").append(reason)} {}
};

}

#endif  // WATERYSQL_VALUE_DECODER_ERROR_H
