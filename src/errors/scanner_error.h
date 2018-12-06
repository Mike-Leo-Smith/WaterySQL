//
// Created by Mike Smith on 2018-12-06.
//

#ifndef WATERYSQL_SCANNER_ERROR_H
#define WATERYSQL_SCANNER_ERROR_H

#include "error.h"
#include "../parsing/token_offset.h"

namespace watery {

struct ScannerError : public Error {
    
    const TokenOffset offset;
    
    explicit ScannerError(std::string_view reason, TokenOffset offset)
        : Error{"ScannerError", reason}, offset{offset} {}
};

}

#endif  // WATERYSQL_SCANNER_ERROR_H
