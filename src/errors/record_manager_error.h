//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_RECORD_MANAGER_ERROR_H
#define WATERYSQL_RECORD_MANAGER_ERROR_H

#include "error.h"

namespace watery {

struct RecordManagerError : public Error {
    explicit RecordManagerError(std::string_view reason) noexcept
        : Error{"RecordManagerError", reason} {}
};

}

#endif  // WATERYSQL_RECORD_MANAGER_ERROR_H
