//
// Created by Mike Smith on 2018-12-03.
//

#ifndef WATERYSQL_SCANNER_H
#define WATERYSQL_SCANNER_H

#include <string>
#include <functional>
#include <unordered_map>
#include "token_tag.h"
#include "token_offset.h"
#include "token.h"
#include "../error/scanner_error.h"

namespace watery {

class Scanner {

public:
    enum struct State : uint32_t {
        READING_BLANK,
        READING_KEYWORD,
        READING_IDENTIFIER,
        READING_STRING,
        READING_NUMBER,
        READING_OPERATOR,
        READING_END
    };

private:
    std::string_view _content;
    Token _lookahead_token{};
    State _state{State::READING_BLANK};
    mutable size_t _curr_pos{0};
    TokenOffset _curr_offset{};
    
    char _peek_char() const;
    char _read_char();
    void _skip_blanks();
    std::string_view _view_content(size_t begin, size_t end) const noexcept;
    void _read_next_token();
    static TokenTag _tag_keyword_or_identifier(std::string_view raw) noexcept;
    static std::string_view _to_upper(std::string_view s) noexcept;

public:
    explicit Scanner(std::string_view content = "");
    Scanner &scan(std::string_view content);
    Scanner &append(std::string_view more);
    TokenTag lookahead() const;
    Token match_token(TokenTag tag);
    void skip();
    TokenOffset current_offset() const;
    bool end() const;
};

}

#endif  // WATERYSQL_SCANNER_H
