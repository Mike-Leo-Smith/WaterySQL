//
// Created by Mike Smith on 2018-12-03.
//

#ifndef WATERYSQL_PARSER_H
#define WATERYSQL_PARSER_H

#include <functional>
#include "scanner.h"
#include "../execution/actor.h"
#include "../data_storage/field_descriptor.h"
#include "../execution/create_table_actor.h"

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
    
    void _parse_field_list(CreateTableActor &actor);
    void _parse_field(CreateTableActor &actor);
    void _parse_foreign_key(CreateTableActor &actor);
    void _parse_primary_key(CreateTableActor &actor);
    void _parse_unique(CreateTableActor &actor);
    DataDescriptor _parse_type();
    bool _parse_nullable_hint();
    uint16_t _parse_size_hint();
    
    int32_t _parse_integer();
    float _parse_float();
    std::string_view _parse_string();
    
public:
    explicit Parser(std::string_view program = "");
    Parser &parse(std::string_view program);
    Parser &append(std::string_view more);
    bool end() const;
    Actor next();
    void skip();
};

}

#endif  // WATERYSQL_PARSER_H
