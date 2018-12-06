//
// Created by Mike Smith on 2018-12-03.
//

#ifndef WATERYSQL_TOKEN_H
#define WATERYSQL_TOKEN_H

#include <string_view>

#include "token_tag.h"
#include "token_offset.h"

namespace watery {

struct Token {
    TokenTag tag;
    std::string_view raw;
    TokenOffset offset;
};

}

#endif  // WATERYSQL_TOKEN_H
