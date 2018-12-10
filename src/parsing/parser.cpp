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

Actor Parser::parse(std::string_view statement) {
    _scanner = Scanner{statement};
    switch (_scanner.lookahead()) {
    case TokenTag::SHOW:
        return _parse_show_statement();
    case TokenTag::CREATE:
        return _parse_create_statement();
    case TokenTag::USE:
        return _parse_use_statement();
    case TokenTag::DROP:
        return _parse_drop_statement();
    case TokenTag::DESC:
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
        throw ParserError{std::string{"Unexpected token \""}.append(token.raw).append("\" after SHOW."), token.offset};
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
    case TokenTag::TABLE: {  // TODO: PRIMARY/FOREIGN keys not supported yet
        _scanner.match_token(TokenTag::TABLE);
        auto identifier = _scanner.match_token(TokenTag::IDENTIFIER);
        _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
        std::array<FieldDescriptor, MAX_FIELD_COUNT> fields;
        auto count = 0u;
        while (true) {
            if (count == MAX_FIELD_COUNT) {
                throw ParserError{
                    std::string{"Failed to create table with more than "}
                        .append(std::to_string(MAX_FIELD_COUNT)).append(" fields."),
                    identifier.offset};
            }
            fields[count++] = _parse_field();
            if (_scanner.lookahead() == TokenTag::RIGHT_PARENTHESIS) { break; }
            _scanner.match_token(TokenTag::COMMA);
        }
        _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
        actor = CreateTableActor{identifier.raw, count, fields};
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
            std::string{"Unexpected token \""}.append(token.raw).append("\" after CREATE."), token.offset};
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
            std::string{"Unexpected token \""}.append(token.raw).append("\" after DROP."), token.offset};
    }
    }
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

Actor Parser::_parse_describe_statement() {
    _scanner.match_token(TokenTag::DESC);
    auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    _scanner.match_token(TokenTag::SEMICOLON);
    return DescribeTableActor{name};
}

FieldDescriptor Parser::_parse_field() {
    switch (_scanner.lookahead()) {
    case TokenTag::IDENTIFIER: {
        auto identifier = _scanner.match_token(TokenTag::IDENTIFIER).raw;
        auto type = _parse_type();
        FieldDescriptor desc{"", type};
        std::strncpy(desc.name, identifier.data(), identifier.size());
        return desc;
    }
    case TokenTag::FOREIGN:  // TODO
        break;
    case TokenTag::PRIMARY:
        break;
    default: {
        auto token = _scanner.match_token(_scanner.lookahead());
        throw ParserError{
            std::string{"Unexpected token \""}.append(token.raw).append("\" in field description."), token.offset};
    }
    }
}

DataDescriptor Parser::_parse_type() {
    TypeTag tag;
    switch (_scanner.lookahead()) {
    case TokenTag::INT:
        _scanner.match_token(TokenTag::INT);
        tag = TypeTag::INTEGER;
        break;
    case TokenTag::VARCHAR:
        _scanner.match_token(TokenTag::VARCHAR);
        tag = TypeTag::VARCHAR;
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
            std::string{"Unexpected token \""}.append(token.raw).append("\" in data type description."), token.offset};
    }
    }
    auto size_hint = static_cast<uint16_t>(_parse_integer());
    auto nullable = _parse_nullable_notation();
    return {tag, nullable, size_hint};
}

bool Parser::_parse_nullable_notation() {
    if (_scanner.lookahead() == TokenTag::NOT) {
        _scanner.match_token(TokenTag::NOT);
        _scanner.match_token(TokenTag::NUL);
        return false;
    }
    _scanner.match_token(TokenTag::NUL);
    return true;
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
            "Failed to convert number literal into integer because of overflow.", token.offset};
    }
    if (end == token.raw.begin()) {
        throw ParserError{
            "Failed to convert number literal into integer because of the mismatched pattern.", token.offset};
    }
    return result;
}

}
