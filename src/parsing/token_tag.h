//
// Created by Mike Smith on 2018-12-03.
//

#ifndef WATERYSQL_TOKEN_TAG_H
#define WATERYSQL_TOKEN_TAG_H

#include <cstdint>
#include "../utility/type/non_trivial_constructible.h"

namespace watery {

enum struct TokenTag : uint32_t {
    // keywords
        DATABASE, DATABASES, TABLE, TABLES, SHOW, CREATE, DROP,
    USE, PRIMARY, KEY, NOT, NUL, INSERT, INTO, VALUES,
    DELETE, FROM, WHERE, UPDATE, SET, SELECT, IS, INT, UNIQUE,
    CHAR, DESCRIBE, REFERENCES, INDEX, AND, DATE, FLOAT, FOREIGN,
    
    // literals
        IDENTIFIER, NUMBER, STRING,  // dates are parsed as strings
    
    // delimiters and operators
        END, SEMICOLON, COMMA, LEFT_PARENTHESIS, RIGHT_PARENTHESIS,
    DOT, EQUAL, UNEQUAL, LESS, LESS_EQUAL, GREATER, GREATER_EQUAL,
    WILDCARD
};

}

#endif  // WATERYSQL_TOKEN_TAG_H
