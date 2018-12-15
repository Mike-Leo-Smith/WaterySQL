//
// Created by Mike Smith on 2018-12-03.
//

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "parser.h"
#include "token_tag_helper.h"
#include "../errors/parser_error.h"

#include "../execution/show_databases_actor.h"
#include "../execution/show_tables_actor.h"
#include "../execution/create_database_actor.h"
#include "../execution/use_database_actor.h"
#include "../execution/drop_database_actor.h"
#include "../execution/create_table_actor.h"
#include "../execution/create_index_actor.h"
#include "../execution/drop_index_actor.h"
#include "../execution/drop_table_actor.h"
#include "../execution/describe_table_actor.h"
#include "../execution/delete_record_actor.h"

#include "../utility/memory/value_decoder.h"
#include "../utility/memory/identifier_comparison.h"
#include "../execution/select_record_actor.h"

namespace watery {

Parser::Parser(std::string_view program)
    : _scanner{program} {}

Parser &Parser::parse(std::string_view program) {
    _scanner.scan(program);
    return *this;
}

Actor Parser::match() {
    switch (_scanner.lookahead()) {
        case TokenTag::SHOW:
            return _parse_show_statement();
        case TokenTag::CREATE:
            return _parse_create_statement();
        case TokenTag::USE:
            return _parse_use_statement();
        case TokenTag::DROP:
            return _parse_drop_statement();
        case TokenTag::DESCRIBE:
            return _parse_describe_statement();
        case TokenTag::INSERT:
            return _parse_insert_statement();
        case TokenTag::DELETE:
            return _parse_delete_statement();
        case TokenTag::UPDATE:
            return _parse_update_statement();
        case TokenTag::SELECT:
            return _parse_select_statement();
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{std::string{"Unexpected command token \""}.append(token.raw).append("\"."), token.offset};
        }
    }
}

Actor Parser::_parse_show_statement() {
    _scanner.match_token(TokenTag::SHOW);
    switch (_scanner.lookahead()) {
        case TokenTag::DATABASES:
            _scanner.match_token(TokenTag::DATABASES);
            _scanner.match_token(TokenTag::SEMICOLON);
            return ShowDatabasesActor{};
        case TokenTag::TABLES:
            _scanner.match_token(TokenTag::TABLES);
            _scanner.match_token(TokenTag::SEMICOLON);
            return ShowTablesActor{};
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Unexpected token \""}.append(token.raw).append(R"(" after "SHOW".)"), token.offset};
        }
    }
}

Actor Parser::_parse_create_statement() {
    _scanner.match_token(TokenTag::CREATE);
    switch (_scanner.lookahead()) {
        case TokenTag::DATABASE: {
            _scanner.match_token(TokenTag::DATABASE);
            auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::SEMICOLON);
            return CreateDatabaseActor{name};
        }
        case TokenTag::TABLE: {
            _scanner.match_token(TokenTag::TABLE);
            auto identifier = _scanner.match_token(TokenTag::IDENTIFIER);
            _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
            CreateTableActor actor{identifier.raw};
            _parse_field_list(actor);
            _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
            _scanner.match_token(TokenTag::SEMICOLON);
            return actor;
        }
        case TokenTag::INDEX: {
            _scanner.match_token(TokenTag::INDEX);
            auto table_name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
            auto column_name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
            _scanner.match_token(TokenTag::SEMICOLON);
            return CreateIndexActor{table_name, column_name};
        }
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Unexpected token \""}.append(token.raw).append(R"(" after "CREATE".)"), token.offset};
        }
    }
}

Actor Parser::_parse_use_statement() {
    _scanner.match_token(TokenTag::USE);
    auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    _scanner.match_token(TokenTag::SEMICOLON);
    return UseDatabaseActor{name};
}

Actor Parser::_parse_drop_statement() {
    _scanner.match_token(TokenTag::DROP);
    switch (_scanner.lookahead()) {
        case TokenTag::DATABASE: {
            _scanner.match_token(TokenTag::DATABASE);
            auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::SEMICOLON);
            return DropDatabaseActor{name};
        }
        case TokenTag::INDEX: {
            _scanner.match_token(TokenTag::INDEX);
            auto tab = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
            auto col = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
            _scanner.match_token(TokenTag::SEMICOLON);
            return DropIndexActor{tab, col};
        }
        case TokenTag::TABLE: {
            _scanner.match_token(TokenTag::TABLE);
            auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::SEMICOLON);
            return DropTableActor{name};
        }
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Unexpected token \""}.append(token.raw).append(R"(" after "DROP".)"), token.offset};
        }
    }
}

Actor Parser::_parse_describe_statement() {
    _scanner.match_token(TokenTag::DESCRIBE);
    auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    _scanner.match_token(TokenTag::SEMICOLON);
    return DescribeTableActor{name};
}

DataDescriptor Parser::_parse_type() {
    switch (_scanner.lookahead()) {
        case TokenTag::INT:
            _scanner.match_token(TokenTag::INT);
            return {TypeTag::INTEGER, _parse_size_hint()};
        case TokenTag::CHAR:
            _scanner.match_token(TokenTag::CHAR);
            return {TypeTag::CHAR, _parse_size_hint()};
        case TokenTag::FLOAT:
            _scanner.match_token(TokenTag::FLOAT);
            return {TypeTag::FLOAT, _parse_size_hint()};
        case TokenTag::DATE:
            _scanner.match_token(TokenTag::DATE);
            return {TypeTag::DATE, _parse_size_hint()};
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Unexpected token \""}.append(token.raw).append("\" in data type description."),
                token.offset};
        }
    }
}

bool Parser::_parse_nullable_hint() {
    if (_scanner.lookahead() == TokenTag::NOT) {
        _scanner.match_token(TokenTag::NOT);
        _scanner.match_token(TokenTag::NUL);
        return false;
    }
    if (_scanner.lookahead() == TokenTag::NUL) {
        _scanner.match_token(TokenTag::NUL);
        return true;
    }
    return _scanner.lookahead() == TokenTag::COMMA ||
           _scanner.lookahead() == TokenTag::RIGHT_PARENTHESIS;
}

std::string_view Parser::_parse_string() {
    thread_local static std::string s;
    s.clear();
    auto token = _scanner.match_token(TokenTag::STRING).raw;
    for (auto iter = token.cbegin(); iter != token.cend(); iter++) {
        s.push_back(*iter == '\\' ? *(++iter) : *iter);
    }
    return s;
}

void Parser::_parse_field_list(CreateTableActor &actor) {
    _parse_field(actor);
    while (_scanner.lookahead() == TokenTag::COMMA) {  // this is a hack to the LL(1) grammar for efficiency
        _scanner.match_token(TokenTag::COMMA);
        if (actor.descriptor.field_count == MAX_FIELD_COUNT) {
            throw ParserError{
                std::string{"Failed to create table with more than "}
                    .append(std::to_string(MAX_FIELD_COUNT)).append(" fields."),
                _scanner.current_offset()};
        }
        _parse_field(actor);
    }
}

void Parser::_parse_field(CreateTableActor &actor) {
    switch (_scanner.lookahead()) {
        case TokenTag::FOREIGN:
            _parse_foreign_key(actor);
            break;
        case TokenTag::PRIMARY:
            _parse_primary_key(actor);
            break;
        case TokenTag::UNIQUE:
            _parse_unique(actor);
            break;
        default: {
            auto identifier = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            auto type = _parse_type();
            auto nullable = _parse_nullable_hint();
            FieldConstraint constraints{nullable ? FieldConstraint::NULLABLE_BIT_MASK : uint8_t{0}};
            actor.descriptor.field_descriptors[actor.descriptor.field_count++] = {identifier, type, constraints};
            break;
        }
    }
}

uint16_t Parser::_parse_size_hint() {
    if (_scanner.lookahead() == TokenTag::LEFT_PARENTHESIS) {
        _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
        auto size_hint_offset = _scanner.current_offset();
        auto size = ValueDecoder::decode_integer(_scanner.match_token(TokenTag::NUMBER).raw);
        _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
        if (size < 0 || size > std::numeric_limits<uint16_t>::max()) {
            throw ParserError{
                std::string{"Invalid size hint \""}.append(std::to_string(size).append("\".")), size_hint_offset};
        }
        return static_cast<uint16_t>(size);
    }
    return 0u;
}

void Parser::_parse_foreign_key(CreateTableActor &actor) {
    _scanner.match_token(TokenTag::FOREIGN);
    _scanner.match_token(TokenTag::KEY);
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto column = _scanner.match_token(TokenTag::IDENTIFIER);
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    _scanner.match_token(TokenTag::REFERENCES);
    auto foreign_table = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto foreign_column = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    for (auto i = 0; i < actor.descriptor.field_count; i++) {
        auto &field = actor.descriptor.field_descriptors[i];
        if (column.raw == field.name) {
            field.constraints.set_foreign(true);
            StringViewCopier::copy(foreign_table, field.foreign_table_name);
            StringViewCopier::copy(foreign_column, field.foreign_column_name);
            return;
        }
    }
    throw ParserError{
        std::string{"Foreign key constraint on undeclared column \""}.append(column.raw).append("\"."), column.offset};
}

void Parser::_parse_primary_key(CreateTableActor &actor) {
    _scanner.match_token(TokenTag::PRIMARY);
    _scanner.match_token(TokenTag::KEY);
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto column = _scanner.match_token(TokenTag::IDENTIFIER);
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    for (auto i = 0; i < actor.descriptor.field_count; i++) {
        auto &field = actor.descriptor.field_descriptors[i];
        if (column.raw == field.name) {
            field.constraints.set_primary(true);
            return;
        }
    }
    throw ParserError{
        std::string{"Primary key constraint on undeclared column \""}.append(column.raw).append("\"."), column.offset};
}

void Parser::_parse_unique(CreateTableActor &actor) {
    _scanner.match_token(TokenTag::UNIQUE);
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto column = _scanner.match_token(TokenTag::IDENTIFIER);
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    for (auto i = 0; i < actor.descriptor.field_count; i++) {
        auto &field = actor.descriptor.field_descriptors[i];
        if (column.raw == field.name) {
            field.constraints.set_unique(true);
            return;
        }
    }
    throw ParserError{
        std::string{"Unique constraint on undeclared column \""}.append(column.raw).append("\"."), column.offset};
}

bool Parser::end() const {
    return _scanner.end();
}

void Parser::skip() {
    while (!_scanner.end()) {
        auto token = _scanner.match_token(_scanner.lookahead());
        if (token.tag == TokenTag::SEMICOLON) {
            break;
        }
    }
}

Parser &Parser::append(std::string_view more) {
    _scanner.append(more);
    return *this;
}

Actor Parser::_parse_insert_statement() {
    _scanner.match_token(TokenTag::INSERT);
    _scanner.match_token(TokenTag::INTO);
    auto target = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    _scanner.match_token(TokenTag::VALUES);
    InsertRecordActor actor{target};
    _parse_value_tuple_list(actor);
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

void Parser::_parse_value_tuple_list(InsertRecordActor &actor) {
    _parse_value_tuple(actor);
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        _parse_value_tuple(actor);
    }
}

void Parser::_parse_value_tuple(InsertRecordActor &actor) {
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    actor.field_lengths.emplace_back(_parse_value(actor.buffer));
    uint16_t field_count = 1;
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        actor.field_lengths.emplace_back(_parse_value(actor.buffer));
        field_count++;
    }
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    actor.field_counts.emplace_back(field_count);
}

uint16_t Parser::_parse_value(std::vector<Byte> &buffer) {
    auto &&encode_value = [&buffer](std::string_view raw) -> uint16_t {
        auto curr_pos = buffer.size();
        if (curr_pos + raw.size() >= buffer.capacity()) {
            buffer.reserve(buffer.capacity() * 2);
        }
        buffer.resize(curr_pos + raw.size());
        StringViewCopier::copy(raw, buffer.data() + curr_pos);
        return static_cast<uint16_t>(raw.size());
    };
    
    switch (_scanner.lookahead()) {
        case TokenTag::NUMBER:      // for INTs and FLOATs
            return encode_value(_scanner.match_token(TokenTag::NUMBER).raw);
        case TokenTag::STRING:      // for CHARs and DATEs
            return encode_value(_parse_string());
        case TokenTag::NUL:
            _scanner.match_token(TokenTag::NUL);
            return 0;
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Token \""}
                    .append(token.raw).append("\" of type \"")
                    .append(TokenTagHelper::name(token.tag))
                    .append("\" cannot be parsed as a VALUE."),
                token.offset
            };
        }
    }
}

void Parser::_parse_column_predicate_operator(ColumnPredicate &predicate) {
    switch (_scanner.lookahead()) {
        case TokenTag::EQUAL:
            _scanner.match_token(TokenTag::EQUAL);
            predicate.op = ColumnPredicateOperator::EQUAL;
            _parse_value(predicate.operand);
            break;
        case TokenTag::UNEQUAL:
            _scanner.match_token(TokenTag::UNEQUAL);
            predicate.op = ColumnPredicateOperator::UNEQUAL;
            _parse_value(predicate.operand);
            break;
        case TokenTag::LESS:
            _scanner.match_token(TokenTag::LESS);
            predicate.op = ColumnPredicateOperator::LESS;
            _parse_value(predicate.operand);
            break;
        case TokenTag::LESS_EQUAL:
            _scanner.match_token(TokenTag::LESS_EQUAL);
            predicate.op = ColumnPredicateOperator::LESS_EQUAL;
            _parse_value(predicate.operand);
            break;
        case TokenTag::GREATER:
            _scanner.match_token(TokenTag::GREATER);
            predicate.op = ColumnPredicateOperator::GREATER;
            _parse_value(predicate.operand);
            break;
        case TokenTag::GREATER_EQUAL:
            _scanner.match_token(TokenTag::GREATER_EQUAL);
            predicate.op = ColumnPredicateOperator::GREATER_EQUAL;
            _parse_value(predicate.operand);
            break;
        case TokenTag::IS:
            _scanner.match_token(TokenTag::IS);
            predicate.op = _parse_column_predicate_null_operator();
            break;
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{std::string{"Unrecognized operator \""}.append(token.raw).append("\"."), token.offset};
        }
    }
}

ColumnPredicateOperator Parser::_parse_column_predicate_null_operator() {
    if (_scanner.lookahead() == TokenTag::NUL) {
        _scanner.match_token(TokenTag::NUL);
        return ColumnPredicateOperator::IS_NULL;
    }
    _scanner.match_token(TokenTag::NOT);
    _scanner.match_token(TokenTag::NUL);
    return ColumnPredicateOperator::NOT_NULL;
}

ColumnPredicate Parser::_parse_column_predicate() {
    ColumnPredicate predicate;
    _parse_column(predicate.table_name, predicate.column_name);
    _parse_column_predicate_operator(predicate);
    return predicate;
}

void Parser::_parse_column(Identifier &table_name, Identifier &column_name) {
    auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    if (_scanner.lookahead() == TokenTag::DOT) {
        _scanner.match_token(TokenTag::DOT);
        StringViewCopier::copy(name, table_name);
        name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    }
    StringViewCopier::copy(name, column_name);
}

void Parser::_parse_where_clause(std::vector<ColumnPredicate> &predicates) {
    if (_scanner.lookahead() != TokenTag::WHERE) { return; }
    _scanner.match_token(TokenTag::WHERE);
    predicates.emplace_back(_parse_column_predicate());
    while (_scanner.lookahead() == TokenTag::AND) {
        _scanner.match_token(TokenTag::AND);
        predicates.emplace_back(_parse_column_predicate());
    }
}

Actor Parser::_parse_delete_statement() {
    _scanner.match_token(TokenTag::DELETE);
    _scanner.match_token(TokenTag::FROM);
    DeleteRecordActor actor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
    _parse_where_clause(actor.predicates);
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

Actor Parser::_parse_update_statement() {
    _scanner.match_token(TokenTag::UPDATE);
    UpdateRecordActor actor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
    _parse_set_clause(actor);
    _parse_where_clause(actor.predicates);
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

void Parser::_parse_set_clause(UpdateRecordActor &actor) {
    auto &&encode_identifier = [&actor](std::string_view id) {
        actor.columns.emplace_back();
        StringViewCopier::copy(id, actor.columns.back().data());
    };
    _scanner.match_token(TokenTag::SET);
    encode_identifier(_scanner.match_token(TokenTag::IDENTIFIER).raw);
    _scanner.match_token(TokenTag::EQUAL);
    actor.lengths.emplace_back(_parse_value(actor.values));
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        encode_identifier(_scanner.match_token(TokenTag::IDENTIFIER).raw);
        _scanner.match_token(TokenTag::EQUAL);
        actor.lengths.emplace_back(_parse_value(actor.values));
    }
}

Actor Parser::_parse_select_statement() {
    _scanner.match_token(TokenTag::SELECT);
    SelectRecordActor actor;
    _parse_selector(actor.selections);
    _scanner.match_token(TokenTag::FROM);
    _parse_selection_table_list(actor.tables);
    _parse_where_clause(actor.predicates);
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

void Parser::_parse_selector(std::vector<Identifier> &sel) {
    if (_scanner.lookahead() == TokenTag::WILDCARD) {
        _scanner.match_token(TokenTag::WILDCARD);
        return;
    }
    sel.emplace_back();
    sel.emplace_back();
    _parse_column(sel[sel.size() - 2], sel[sel.size() - 1]);
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        sel.emplace_back();
        sel.emplace_back();
        _parse_column(sel[sel.size() - 2], sel[sel.size() - 1]);
    }
}

void Parser::_parse_selection_table_list(std::vector<Identifier> &tables) {
    auto &&encode_identifier = [&tables](std::string_view id) {
        tables.emplace_back();
        StringViewCopier::copy(id, tables.back().data());
    };
    encode_identifier(_scanner.match_token(TokenTag::IDENTIFIER).raw);
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        encode_identifier(_scanner.match_token(TokenTag::IDENTIFIER).raw);
    }
}

}
