//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_PAGE_MANAGER_ERROR_H
#define WATERYSQL_PAGE_MANAGER_ERROR_H

#include "error.h"

namespace watery {

struct PageManagerError : public Error {
    explicit PageManagerError(std::string_view reason) noexcept
        : Error{"PageManagerError", reason} {}
};

}

#endif  // WATERYSQL_PAGE_MANAGER_ERROR_H
