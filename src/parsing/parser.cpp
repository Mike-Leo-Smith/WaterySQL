//
// Created by Mike Smith on 2018-12-03.
//

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "parser.h"
#include "../execution/show_databases_actor.h"
#include "../errors/parser_error.h"
#include "../execution/show_tables_actor.h"
#include "../execution/create_database_actor.h"
#include "../execution/use_database_actor.h"
#include "../execution/drop_database_actor.h"
#include "../execution/create_table_actor.h"
#include "../execution/create_index_actor.h"
#include "../execution/drop_index_actor.h"
#include "../execution/drop_table_actor.h"
#include "../execution/describe_table_actor.h"

namespace watery {

Parser::Parser(std::string_view program)
    : _scanner{program} {}

Parser &Parser::parse(std::string_view program) {
    _scanner.scan(program);
    return *this;
}

Actor Parser::next() {
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
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{std::string{"Unexpected command token \""}.append(token.raw).append("\"."), token.offset};
        }
    }
}

Actor Parser::_parse_show_statement() {
    _scanner.match_token(TokenTag::SHOW);
    Actor actor;
    switch (_scanner.lookahead()) {
        case TokenTag::DATABASES:
            _scanner.match_token(TokenTag::DATABASES);
            actor = ShowDatabasesActor{};
            break;
        case TokenTag::TABLES:
            _scanner.match_token(TokenTag::TABLES);
            actor = ShowTablesActor{};
            break;
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Unexpected token \""}.append(token.raw).append(R"(" after "SHOW".)"), token.offset};
        }
    }
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

Actor Parser::_parse_create_statement() {
    _scanner.match_token(TokenTag::CREATE);
    Actor actor;
    switch (_scanner.lookahead()) {
        case TokenTag::DATABASE:
            _scanner.match_token(TokenTag::DATABASE);
            actor = CreateDatabaseActor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
            break;
        case TokenTag::TABLE: {
            _scanner.match_token(TokenTag::TABLE);
            auto identifier = _scanner.match_token(TokenTag::IDENTIFIER);
            _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
            thread_local static std::vector<FieldDescriptor> fields;
            fields.clear();
            _parse_field_list(fields);
            _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
            actor = CreateTableActor{identifier.raw, fields};
            break;
        }
        case TokenTag::INDEX: {
            _scanner.match_token(TokenTag::INDEX);
            auto table_name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
            auto column_name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
            actor = CreateIndexActor{table_name, column_name};
            break;
        }
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Unexpected token \""}.append(token.raw).append(R"(" after "CREATE".)"), token.offset};
        }
    }
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

Actor Parser::_parse_use_statement() {
    _scanner.match_token(TokenTag::USE);
    UseDatabaseActor actor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

Actor Parser::_parse_drop_statement() {
    _scanner.match_token(TokenTag::DROP);
    Actor actor;
    switch (_scanner.lookahead()) {
        case TokenTag::DATABASE:
            _scanner.match_token(TokenTag::DATABASE);
            actor = DropDatabaseActor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
            break;
        case TokenTag::INDEX: {
            _scanner.match_token(TokenTag::INDEX);
            auto tab = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
            auto col = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
            actor = DropIndexActor{tab, col};
            break;
        }
        case TokenTag::TABLE:
            _scanner.match_token(TokenTag::TABLE);
            actor = DropTableActor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
            break;
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Unexpected token \""}.append(token.raw).append(R"(" after "DROP".)"), token.offset};
        }
    }
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

Actor Parser::_parse_describe_statement() {
    _scanner.match_token(TokenTag::DESCRIBE);
    auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    _scanner.match_token(TokenTag::SEMICOLON);
    return DescribeTableActor{name};
}

DataDescriptor Parser::_parse_type() {
    TypeTag tag;
    switch (_scanner.lookahead()) {
        case TokenTag::INT:
            _scanner.match_token(TokenTag::INT);
            tag = TypeTag::INTEGER;
            break;
        case TokenTag::CHAR:
            _scanner.match_token(TokenTag::CHAR);
            tag = TypeTag::CHAR;
            break;
        case TokenTag::FLOAT:
            _scanner.match_token(TokenTag::FLOAT);
            tag = TypeTag::FLOAT;
            break;
        case TokenTag::DATE:
            _scanner.match_token(TokenTag::DATE);
            tag = TypeTag::DATE;
            break;
        default: {
            auto token = _scanner.match_token(_scanner.lookahead());
            throw ParserError{
                std::string{"Unexpected token \""}.append(token.raw).append("\" in data type description."),
                token.offset};
        }
    }
    auto size_hint = _parse_size_hint();
    return {tag, size_hint};
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

int32_t Parser::_parse_integer() {
    int32_t result;
    auto token = _scanner.match_token(TokenTag::NUMBER);
    auto ec = std::from_chars(token.raw.begin(), token.raw.end(), result).ec;
    if (ec == std::errc::invalid_argument) {
        throw ParserError{
            "Failed to convert number literal into integer because of the mismatched pattern.", token.offset};
    }
    if (ec == std::errc::result_out_of_range) {
        throw ParserError{
            "Failed to convert number literal into integer because of overflow.", token.offset};
    }
    return result;
}

float Parser::_parse_float() {
    auto token = _scanner.match_token(TokenTag::NUMBER);
    char *end;
    auto result = std::strtof(token.raw.begin(), &end);
    if (result == HUGE_VALF) {
        throw ParserError{
            "Failed to convert number literal into float because of overflow.", token.offset};
    }
    if (end == token.raw.begin()) {
        throw ParserError{
            "Failed to convert number literal into float because of the mismatched pattern.", token.offset};
    }
    return result;
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

void Parser::_parse_field_list(std::vector<FieldDescriptor> &fields) {
    _parse_field(fields);
    while (_scanner.lookahead() == TokenTag::COMMA) {  // this is a hack to the LL(1) grammar for efficiency
        _scanner.match_token(TokenTag::COMMA);
        if (fields.size() == MAX_FIELD_COUNT) {
            throw ParserError{
                std::string{"Failed to create table with more than "}
                    .append(std::to_string(MAX_FIELD_COUNT)).append(" fields."),
                _scanner.current_offset()};
        }
        _parse_field(fields);
    }
}

void Parser::_parse_field(std::vector<FieldDescriptor> &fields) {
    switch (_scanner.lookahead()) {
        case TokenTag::FOREIGN:
            _parse_foreign_key(fields);
            break;
        case TokenTag::PRIMARY:
            _parse_primary_key(fields);
            break;
        case TokenTag::UNIQUE:
            _parse_unique(fields);
            break;
        default: {
            auto identifier = _scanner.match_token(TokenTag::IDENTIFIER).raw;
            auto type = _parse_type();
            auto nullable = _parse_nullable_hint();
            FieldConstraint constraints{nullable ? FieldConstraint::NULLABLE_BIT_MASK : uint8_t{0}};
            fields.emplace_back(identifier, type, constraints);
            break;
        }
    }
}

uint16_t Parser::_parse_size_hint() {
    if (_scanner.lookahead() == TokenTag::LEFT_PARENTHESIS) {
        _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
        auto size_hint_offset = _scanner.current_offset();
        auto size = _parse_integer();
        _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
        if (size < 0 || size > std::numeric_limits<uint16_t>::max()) {
            throw ParserError{
                std::string{"Invalid size hint \""}.append(std::to_string(size).append("\".")), size_hint_offset};
        }
        return static_cast<uint16_t>(size);
    }
    return 0u;
}

void Parser::_parse_foreign_key(std::vector<FieldDescriptor> &fields) {
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
    for (auto &&field: fields) {
        if (column.raw == field.name) {
            field.constraints.set_foreign(true);
            foreign_table.copy(field.foreign_table_name, foreign_table.size());
            foreign_column.copy(field.foreign_column_name, foreign_column.size());
            return;
        }
    }
    throw ParserError{
        std::string{"Foreign key constraint on undeclared column \""}.append(column.raw).append("\"."), column.offset};
}

void Parser::_parse_primary_key(std::vector<FieldDescriptor> &fields) {
    _scanner.match_token(TokenTag::PRIMARY);
    _scanner.match_token(TokenTag::KEY);
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto column = _scanner.match_token(TokenTag::IDENTIFIER);
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    for (auto &&field: fields) {
        if (column.raw == field.name) {
            field.constraints.set_primary(true);
            return;
        }
    }
    throw ParserError{
        std::string{"Primary key constraint on undeclared column \""}.append(column.raw).append("\"."), column.offset};
}

void Parser::_parse_unique(std::vector<FieldDescriptor> &fields) {
    _scanner.match_token(TokenTag::UNIQUE);
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto column = _scanner.match_token(TokenTag::IDENTIFIER);
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    for (auto &&field: fields) {
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

}
