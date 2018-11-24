//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_ERROR_H
#define WATERYSQL_ERROR_H

#include <exception>

namespace watery {

struct Error : public std::exception {
    
    const char *what() const noexcept override {
        return "Unknown error";
    }
    
};

}

#endif  // WATERYSQL_ERROR_H
