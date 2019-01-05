//
// Created by Mike Smith on 2018-12-03.
//

#include <iostream>
#include "scanner.h"
#include "../config/config.h"
#include "token_tag_helper.h"
#include "../utility/io/error_printer.h"

namespace watery {

std::string_view Scanner::_to_upper(std::string_view s) noexcept {
    thread_local static std::string upper_cased;
    upper_cased.resize(s.size());
    std::transform(s.cbegin(), s.cend(), upper_cased.begin(), [](char c) { return static_cast<char>(toupper(c)); });
    return upper_cased;
}

TokenTag Scanner::_tag_keyword_or_identifier(std::string_view raw) noexcept {
    auto upper_cased = _to_upper(raw);
    if (TokenTagHelper::keyword_dict().count(upper_cased) != 0) {
        return TokenTagHelper::keyword_dict().at(upper_cased);
    }
    return TokenTag::IDENTIFIER;
}

char Scanner::_peek_char() const {
    auto c = _content[_curr_pos];
    if (c == '\n' && _content[_curr_pos + 1] == '\r') { c = _content[_curr_pos++]; }
    if (c == '\r') { c = '\n'; }
    if (c == EOF) { c = '\0'; }
    return c;
}

char Scanner::_read_char() {
    auto c = _peek_char();
    if (c == '\0') { return c; }
    _curr_pos++;
    _curr_offset.column++;
    if (c == '\n') {
        _curr_offset.line++;
        _curr_offset.column = 0;
    }
    return c;
}

std::string_view Scanner::_view_content(size_t begin, size_t end) const noexcept {
    return _content.substr(begin, end - begin);
}

void Scanner::_read_next_token() {
    
    // scanning reaches end, no more moves
    if (_state == State::READING_END) {
        return;
    }
    
    // skip blanks
    _skip_blanks();
    _lookahead_token.offset = _curr_offset;
    
    // read end
    if (_peek_char() == '\0') {
        _lookahead_token.tag = TokenTag::END;
        _lookahead_token.raw = _view_content(_curr_pos, _curr_pos);
        _state = State::READING_END;
        return;
    }
    
    auto last_pos = _curr_pos;
    auto c = _read_char();
    
    // read numbers
    if (std::isdigit(c) || c == '-') {
        if (_state != State::READING_BLANK && _state != State::READING_OPERATOR) {
            throw ScannerError{"Unexpected digit.", _curr_offset};
        }
        while (std::isdigit(_peek_char())) { _read_char(); }
        if (_peek_char() == '.') { _read_char(); }
        while (std::isdigit(_peek_char())) { _read_char(); }
        _lookahead_token.raw = _view_content(last_pos, _curr_pos);
        _lookahead_token.tag = TokenTag::NUMBER;
        _state = State::READING_NUMBER;
        return;
    }
    
    // read wildcard
    if (c == '*') {
        if (_state != State::READING_BLANK && _state != State::READING_OPERATOR) {
            throw ScannerError{"Unexpected wild card.", _curr_offset};
        }
        _lookahead_token.raw = _view_content(last_pos, _curr_pos);
        _lookahead_token.tag = TokenTag::WILDCARD;
        _state = State::READING_KEYWORD;  // for simplification, wildcards share the state with keywords.
        return;
    }
    
    // read keywords or identifiers
    if (std::isalpha(c) || c == '`') {
        if (_state != State::READING_BLANK && _state != State::READING_OPERATOR) {
            throw ScannerError{"Unexpected alpha/underscore.", _curr_offset};
        }
        while (std::isalnum(_peek_char()) || _peek_char() == '_') { _read_char(); }
        if (c == '`') {
            if (_peek_char() != '`') {
                throw ScannerError{"Missing closing backtick for identifier.", _curr_offset};
            }
            _lookahead_token.raw = _view_content(last_pos + 1, _curr_pos);
            _read_char();  // eat "`"
        } else {
            _lookahead_token.raw = _view_content(last_pos, _curr_pos);
        }
        if (auto l = _lookahead_token.raw.size(); l == 0 || l > MAX_IDENTIFIER_LENGTH) {
            throw ScannerError{
                std::string{"Bad identifier name length "}.append(std::to_string(l)).append("."), _curr_offset};
        }
        _lookahead_token.tag = _tag_keyword_or_identifier(_lookahead_token.raw);
        _state = (_lookahead_token.tag == TokenTag::IDENTIFIER) ?
                 State::READING_IDENTIFIER :
                 State::READING_KEYWORD;
        return;
    }
    
    // read string
    if (c == '\'' || c == '"') {
        if (_state != State::READING_BLANK && _state != State::READING_OPERATOR) {
            throw ScannerError{"Unexpected quotation.", _curr_offset};
        }
        while (_peek_char() != c) {
            auto x = _read_char();
            if (x == '\\') {  // skip escapes
                _read_char();
            }
            if (x == '\n') {
                _state = State::READING_BLANK;
                throw ScannerError{"Strings cannot be contain raw line breaks, use '\\n' instead.", _curr_offset};
            }
            if (x == '\0') {
                _state = State::READING_END;
                throw ScannerError{"Unexpected end.", _curr_offset};
            }
        }
        _read_char();  // read closing quotation.
        _lookahead_token.raw = _view_content(last_pos + 1, _curr_pos - 1);
        _lookahead_token.tag = TokenTag::STRING;
        _state = State::READING_STRING;
        return;
    }
    
    // read operators or delimiters
    switch (c) {
        case '.':
            _lookahead_token.tag = TokenTag::DOT;
            break;
        case '<': {
            if (_peek_char() == '>') {
                _read_char();
                _lookahead_token.tag = TokenTag::UNEQUAL;
            } else if (_peek_char() == '=') {
                _read_char();
                _lookahead_token.tag = TokenTag::LESS_EQUAL;
            } else {
                _lookahead_token.tag = TokenTag::LESS;
            }
            break;
        }
        case '>': {
            if (_peek_char() == '=') {
                _read_char();
                _lookahead_token.tag = TokenTag::GREATER_EQUAL;
            } else {
                _lookahead_token.tag = TokenTag::GREATER;
            }
            break;
        }
        case '=':
            _lookahead_token.tag = TokenTag::EQUAL;
            break;
        case ',':
            _lookahead_token.tag = TokenTag::COMMA;
            break;
        case ';':
            _lookahead_token.tag = TokenTag::SEMICOLON;
            break;
        case '(':
            _lookahead_token.tag = TokenTag::LEFT_PARENTHESIS;
            break;
        case ')':
            _lookahead_token.tag = TokenTag::RIGHT_PARENTHESIS;
            break;
        default:
            throw ScannerError(
                std::string{"Unexpected character \""}.append(std::string_view{&c, 1}).append("\"."), _curr_offset);
    }
    
    _lookahead_token.raw = _view_content(last_pos, _curr_pos);
    _state = State::READING_OPERATOR;
    
}

void Scanner::_skip_blanks() {
    while (std::isblank(_peek_char()) || _peek_char() == '\n') {
        _read_char();
        _state = State::READING_BLANK;
    }
}

TokenTag Scanner::lookahead() const {
    return _lookahead_token.tag;
}

Token Scanner::match_token(TokenTag tag) {
    if (_lookahead_token.tag != tag) {
        if (tag == TokenTag::IDENTIFIER &&
            TokenTagHelper::keyword_dict().count(_to_upper(_lookahead_token.raw)) != 0) {
            _lookahead_token.tag = TokenTag::IDENTIFIER;
        } else {
            throw ScannerError{
                std::string{"Current token \""}
                    .append(_lookahead_token.raw).append("\" does not match the expected token \"")
                    .append(TokenTagHelper::name(tag)).append("\"."),
                _lookahead_token.offset};
        }
    }
    auto token = _lookahead_token;
    _read_next_token();
    return token;
}

Scanner::Scanner(std::string_view content)
    : _content{content} {
    _read_next_token();
}

Scanner &Scanner::scan(std::string_view content) {
    _content = content;
    _state = State::READING_BLANK;
    _curr_offset = {};
    _curr_pos = 0;
    _read_next_token();
    return *this;
}

TokenOffset Scanner::current_offset() const {
    return _curr_offset;
}

bool Scanner::end() const {
    return _lookahead_token.tag == TokenTag::END;
}

Scanner &Scanner::append(std::string_view more) {
    _content = more;
    _state = State::READING_BLANK;
    _read_next_token();
    return *this;
}

}
