//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_FILE_IO_ERROR_H
#define WATERYSQL_FILE_IO_ERROR_H

#include <string>
#include "error.h"

namespace watery {

class FileIOError : public Error {

private:
    std::string _message{"File I/O Error: "};

public:
    explicit FileIOError(std::string_view reason) { _message += reason; }
    const char *what() const noexcept override { return _message.c_str(); }
};

}

#endif  // WATERYSQL_FILE_IO_ERROR_H
