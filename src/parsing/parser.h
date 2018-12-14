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
    
    void _parse_field_list(std::vector<FieldDescriptor> &fields);
    void _parse_field(std::vector<FieldDescriptor> &fields);
    void _parse_foreign_key(std::vector<FieldDescriptor> &fields);
    void _parse_primary_key(std::vector<FieldDescriptor> &fields);
    void _parse_unique(std::vector<FieldDescriptor> &fields);
    DataDescriptor _parse_type();
    bool _parse_nullable_hint();
    uint16_t _parse_size_hint();
    
    int32_t _parse_integer();
    float _parse_float();
    std::string_view _parse_string();
    
public:
    explicit Parser(std::string_view program = "");
    Parser &parse(std::string_view program);
    bool end() const;
    Actor next();
    void skip();
};

}

#endif  // WATERYSQL_PARSER_H
