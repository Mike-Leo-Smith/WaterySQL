//
// Created by Mike Smith on 2018-12-13.
//

#ifndef WATERYSQL_TOKEN_TAG_HELPER_H
#define WATERYSQL_TOKEN_TAG_HELPER_H

#include <string_view>
#include <unordered_map>


#include "../utility/type/non_constructible.h"
#include "token_tag.h"

namespace watery {

struct TokenTagHelper : NonConstructible {
    
    static const std::unordered_map<std::string_view, TokenTag> &keyword_dict() noexcept;
    static std::string_view name(TokenTag tag) noexcept;
    
};

}

#endif  // WATERYSQL_TOKEN_TAG_HELPER_H
