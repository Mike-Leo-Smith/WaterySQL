//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_PAGE_MANAGER_ERROR_H
#define WATERYSQL_PAGE_MANAGER_ERROR_H

#include <string>
#include "error.h"

namespace watery {

class PageManagerError : public Error {

private:
    std::string _message{"Page Manager Error: "};

public:
    explicit PageManagerError(std::string_view reason) { _message.append(reason); }
    const char *what() const noexcept override { return _message.c_str(); }
};

}

#endif  // WATERYSQL_PAGE_MANAGER_ERROR_H
