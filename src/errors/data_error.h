//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_DATA_ERROR_H
#define WATERYSQL_DATA_ERROR_H

#include "error.h"

namespace watery {

struct DataError : public Error {
    explicit DataError(std::string_view reason)
        : Error{"DataError", reason} {}
};

}

#endif  // WATERYSQL_DATA_ERROR_H
