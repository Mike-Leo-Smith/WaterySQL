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
#include "token_tag_helper.h"

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
            foreign_table.copy(field.foreign_table_name, foreign_table.size());
            foreign_column.copy(field.foreign_column_name, foreign_column.size());
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
    _parse_value(actor);
    uint16_t field_count = 1;
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        _parse_value(actor);
        field_count++;
    }
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    actor.field_counts.emplace_back(field_count);
}

void Parser::_parse_value(InsertRecordActor &actor) {
    
    auto &&encode_value = [&actor](std::string_view raw) {
        auto curr_pos = actor.buffer.size();
        if (curr_pos + raw.size() >= actor.buffer.capacity()) {
            actor.buffer.reserve(actor.buffer.capacity() * 2);
        }
        actor.buffer.resize(curr_pos + raw.size());
        raw.copy(actor.buffer.data() + curr_pos, raw.size());
        actor.field_lengths.emplace_back(raw.size());
    };
    
    switch (_scanner.lookahead()) {
        case TokenTag::NUMBER: {    // for INTs and FLOATs
            encode_value(_scanner.match_token(TokenTag::NUMBER).raw);
            break;
        }
        case TokenTag::STRING: {    // for CHARs and DATEs
            encode_value(_parse_string());
            break;
        }
        case TokenTag::NUL: {
            _scanner.match_token(TokenTag::NUL);
            actor.field_lengths.emplace_back(0u);
            break;
        }
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

}
