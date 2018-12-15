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
#include "../execution/insert_record_actor.h"
#include "../execution/column_predicate.h"

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
    Actor _parse_insert_statement();
    Actor _parse_delete_statement();
    
    void _parse_field_list(CreateTableActor &actor);
    void _parse_field(CreateTableActor &actor);
    void _parse_foreign_key(CreateTableActor &actor);
    void _parse_primary_key(CreateTableActor &actor);
    void _parse_unique(CreateTableActor &actor);
    DataDescriptor _parse_type();
    bool _parse_nullable_hint();
    uint16_t _parse_size_hint();
    
    void _parse_value_tuple_list(InsertRecordActor &actor);
    void _parse_value_tuple(InsertRecordActor &actor);
    
    uint16_t _parse_value(std::vector<Byte> &buffer);
    std::string_view _parse_string();
    void _parse_column_predicate_operator(ColumnPredicate &predicate);
    ColumnPredicateOperator _parse_column_predicate_null_operator();
    ColumnPredicate _parse_column_predicate();
    void _parse_column(char *table_name, char *column_name);
    std::vector<ColumnPredicate> _parse_where_clause();
    
public:
    explicit Parser(std::string_view program = "");
    Parser &parse(std::string_view program);
    Parser &append(std::string_view more);
    bool end() const;
    Actor match();
    void skip();
};

}

#endif  // WATERYSQL_PARSER_H
