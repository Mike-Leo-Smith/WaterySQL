//
// Created by Mike Smith on 2018-12-03.
//

#ifndef WATERYSQL_PARSER_H
#define WATERYSQL_PARSER_H

#include <functional>
#include "scanner.h"
#include "../action/actor.h"
#include "../data/field_descriptor.h"
#include "../action/create_table_actor.h"
#include "../action/insert_record_actor.h"
#include "../query/column_predicate.h"
#include "../action/update_record_actor.h"

namespace watery {

class Parser {
private:
    Scanner _scanner;

protected:
    Actor _parse_show_statement();
    Actor _parse_use_statement();
    Actor _parse_drop_statement();
    Actor _parse_create_statement();
    Actor _parse_insert_statement();
    Actor _parse_delete_statement();
    Actor _parse_update_statement();
    Actor _parse_select_statement();
    Actor _parse_describe_statement();
    
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
    void _parse_column(Identifier &table_name, Identifier &column_name);
    void _parse_where_clause(std::vector<ColumnPredicate> &predicates);
    void _parse_set_clause(UpdateRecordActor &actor);
    void _parse_selector(std::vector<Identifier> &sel);
    void _parse_selection_table_list(std::vector<Identifier> &tables);
    
public:
    explicit Parser(std::string_view program = "");
    Parser &parse(std::string_view program);
    Parser &append(std::string_view more);
    bool end() const;
    Actor match();
    void skip();
    Actor _parse_exit_statement();
};

}

#endif  // WATERYSQL_PARSER_H
