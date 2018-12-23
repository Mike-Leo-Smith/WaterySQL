//
// Created by Mike Smith on 2018-12-06.
//

#ifndef WATERYSQL_SYSTEM_MANAGER_ERROR_H
#define WATERYSQL_SYSTEM_MANAGER_ERROR_H

#include "error.h"

namespace watery {

struct SystemManagerError : public Error {
    
    explicit SystemManagerError(std::string_view reason) noexcept
        : Error("SystemManagerError", reason) {}
    
};

}

#endif  // WATERYSQL_SYSTEM_MANAGER_ERROR_H
