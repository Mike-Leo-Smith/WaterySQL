//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_RECORD_MANAGER_ERROR_H
#define WATERYSQL_RECORD_MANAGER_ERROR_H

#include <string>
#include "error.h"

namespace watery {

class RecordManagerError : public Error {
private:
    std::string _message{"Record Manager Error: "};

public:
    explicit RecordManagerError(std::string_view reason) { _message.append(reason); }
    const char *what() const noexcept override { return _message.c_str(); }
};

}

#endif  // WATERYSQL_RECORD_MANAGER_ERROR_H
