//
// Created by Mike Smith on 2018-12-03.
//

#ifndef WATERYSQL_PARSER_H
#define WATERYSQL_PARSER_H

#include <functional>
#include "scanner.h"
#include "../execution/actor.h"
#include "../data_storage/field_descriptor.h"

namespace watery {

class Parser {
private:
    Scanner _scanner;

protected:
    Actor _parse_show_statement();
    Actor _parse_create_statement();
    Actor _parse_use_statement();
    Actor _parse_drop_statement();
    Actor _parse_describe_statement();
    
    FieldDescriptor _parse_field();
    DataDescriptor _parse_type();
    bool _parse_nullable_notation();
    int32_t _parse_integer();
    float _parse_float();
    
public:
    Actor parse(std::string_view statement);
};

}

#endif  // WATERYSQL_PARSER_H
