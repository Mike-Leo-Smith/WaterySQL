//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_PARSER_ERROR_H
#define WATERYSQL_PARSER_ERROR_H

#include "error.h"
#include "../parsing/token_offset.h"

namespace watery {

struct ParserError : public Error {
    
    const TokenOffset offset;
    
    ParserError(std::string_view reason, TokenOffset offset)
        : Error("ParserError", reason), offset{offset} {}
        
};

}

#endif  // WATERYSQL_PARSER_ERROR_H
