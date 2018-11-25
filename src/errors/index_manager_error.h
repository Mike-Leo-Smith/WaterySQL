//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_MANAGER_ERROR_H
#define WATERYSQL_INDEX_MANAGER_ERROR_H

#include "error.h"

namespace watery {

struct IndexManagerError : public Error {
    explicit IndexManagerError(std::string_view reason)
        : Error{"IndexManagerError", reason} {}
};

}

#endif  // WATERYSQL_INDEX_MANAGER_ERROR_H
