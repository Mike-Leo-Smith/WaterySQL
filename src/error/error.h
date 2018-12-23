//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_ERROR_H
#define WATERYSQL_ERROR_H

#include <exception>
#include <string>

namespace watery {

class Error : public std::exception {

private:
    std::string _message;
    
public:
    Error(std::string_view type, std::string_view reason) noexcept {
        _message.append(type).append(": ").append(reason);
    }
    
    const char *what() const noexcept override {
        return _message.c_str();
    }
    
};

}

#endif  // WATERYSQL_ERROR_H
