//
// Created by Mike Smith on 2018-12-03.
//

#include <charconv>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include "parser.h"
#include "token_tag_helper.h"
#include "../error/parser_error.h"

#include "../action/show_databases_actor.h"
#include "../action/show_tables_actor.h"
#include "../action/create_database_actor.h"
#include "../action/use_database_actor.h"
#include "../action/drop_database_actor.h"
#include "../action/create_table_actor.h"
#include "../action/create_index_actor.h"
#include "../action/drop_index_actor.h"
#include "../action/drop_table_actor.h"
#include "../action/describe_table_actor.h"
#include "../action/delete_record_actor.h"

#include "../utility/memory/value_decoder.h"
#include "../utility/memory/identifier_comparison.h"
#include "../action/select_record_actor.h"
#include "../action/exit_actor.h"
#include "../action/execute_file_actor.h"
#include "../action/commit_actor.h"

namespace watery {

Parser::Parser(std::string_view program)
    : _scanner{program} {}

Parser &Parser::parse(std::string_view program) {
    _scanner.scan(program);
    return *this;
}

Actor Parser::match() {
    switch (_scanner.lookahead()) {
        case TokenTag::SEMICOLON:
            return [] {};
        case TokenTag::EXIT:
            return _parse_exit_statement();
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
        case TokenTag::SOURCE:
            return _parse_execute_statement();
        case TokenTag::COMMIT:
            return _parse_commit_statement();
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
    s.push_back('\'');
    auto token = _scanner.match_token(TokenTag::STRING).raw;
    for (auto iter = token.cbegin(); iter != token.cend(); iter++) {
        s.push_back(*iter == '\\' ? *(++iter) : *iter);
    }
    s.push_back('\'');
    return s;
}

void Parser::_parse_field_list(CreateTableActor &actor) {
    actor.descriptor.length = 0u;
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
    actor.descriptor.length = 0;
    if (actor.descriptor.null_mapped) {
        actor.descriptor.length += sizeof(NullFieldBitmap);
    }
    if (actor.descriptor.reference_counted) {
        actor.descriptor.length += sizeof(uint32_t);
    }
    for (auto i = 0; i < actor.descriptor.field_count; i++) {
        actor.descriptor.field_offsets[i] = actor.descriptor.length;
        actor.descriptor.length += actor.descriptor.field_descriptors[i].data_descriptor.length;
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
            if (nullable) {
                actor.descriptor.null_mapped = true;
            }
            FieldConstraint constraints{nullable ? FieldConstraint::NULLABLE_BIT_MASK : FieldConstraint::Mask{0}};
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
    auto foreign_table = _scanner.match_token(TokenTag::IDENTIFIER);
    
    if (foreign_table.raw == actor.name) {
        throw ParserError{
            std::string{"Foregin keys cannot refer to table \""}.append(actor.name).append("\" itself."),
            foreign_table.offset};
    }
    
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto foreign_column = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    
    auto foreign_col_offset = actor.descriptor.get_column_offset(column.raw);
    auto &field = actor.descriptor.field_descriptors[foreign_col_offset];
    if (field.constraints.foreign()) {
        throw ParserError{
            std::string{"Cannot have multiple check/foreign key constraints on column \""}
                .append(column.raw).append("\""),
            column.offset};
    }
    field.constraints.set_foreign();
    actor.descriptor.foreign_referencing = true;
    StringViewCopier::copy(foreign_table.raw, field.foreign_table_name);
    StringViewCopier::copy(foreign_column, field.foreign_column_name);
}

void Parser::_parse_primary_key(CreateTableActor &actor) {
    _scanner.match_token(TokenTag::PRIMARY);
    _scanner.match_token(TokenTag::KEY);
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto column = _scanner.match_token(TokenTag::IDENTIFIER);
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    
    if (actor.descriptor.primary_key_column_offset != -1) {
        throw ParserError{"Primary key constraint cannot be set on multiple columns in one table.", column.offset};
    }
    auto primary_col_offset = actor.descriptor.get_column_offset(column.raw);
    if (actor.descriptor.field_descriptors[primary_col_offset].constraints.nullable()) {
        throw ParserError{"Cannot add primary key constraint on a nullable column.", column.offset};
    }
    actor.descriptor.reference_counted = true;
    actor.descriptor.field_descriptors[primary_col_offset].constraints.set_primary();
    actor.descriptor.primary_key_column_offset = primary_col_offset;
}

void Parser::_parse_unique(CreateTableActor &actor) {
    _scanner.match_token(TokenTag::UNIQUE);
    _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
    auto column = _scanner.match_token(TokenTag::IDENTIFIER);
    _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    
    auto unique_col_offset = actor.descriptor.get_column_offset(column.raw);
    auto &field = actor.descriptor.field_descriptors[unique_col_offset];
    if (field.constraints.unique()) {
        throw ParserError{
            std::string{"Column \""}.append(column.raw).append("\" has already be set unique."), column.offset};
    }
    field.constraints.set_unique();
}

bool Parser::end() const {
    return _scanner.end();
}

void Parser::skip() noexcept {
    while (!_scanner.end()) {
        try {
            auto token = _scanner.match_token(_scanner.lookahead());
            if (token.tag == TokenTag::SEMICOLON) { break; }
        } catch (const std::exception &e) {
            print_error(std::cerr, e);
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
    actor.field_sizes.emplace_back(_parse_value(actor.buffer));
    uint16_t field_count = 1;
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        actor.field_sizes.emplace_back(_parse_value(actor.buffer));
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
            predicate.op = PredicateOperator::EQUAL;
            _parse_column_predicate_operand(predicate);
            break;
        case TokenTag::UNEQUAL:
            _scanner.match_token(TokenTag::UNEQUAL);
            predicate.op = PredicateOperator::UNEQUAL;
            _parse_column_predicate_operand(predicate);
            break;
        case TokenTag::LESS:
            _scanner.match_token(TokenTag::LESS);
            predicate.op = PredicateOperator::LESS;
            _parse_column_predicate_operand(predicate);
            break;
        case TokenTag::LESS_EQUAL:
            _scanner.match_token(TokenTag::LESS_EQUAL);
            predicate.op = PredicateOperator::LESS_EQUAL;
            _parse_column_predicate_operand(predicate);
            break;
        case TokenTag::GREATER:
            _scanner.match_token(TokenTag::GREATER);
            predicate.op = PredicateOperator::GREATER;
            _parse_column_predicate_operand(predicate);
            break;
        case TokenTag::GREATER_EQUAL:
            _scanner.match_token(TokenTag::GREATER_EQUAL);
            predicate.op = PredicateOperator::GREATER_EQUAL;
            _parse_column_predicate_operand(predicate);
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

PredicateOperator Parser::_parse_column_predicate_null_operator() {
    if (_scanner.lookahead() == TokenTag::NUL) {
        _scanner.match_token(TokenTag::NUL);
        return PredicateOperator::IS_NULL;
    }
    _scanner.match_token(TokenTag::NOT);
    _scanner.match_token(TokenTag::NUL);
    return PredicateOperator::NOT_NULL;
}

ColumnPredicate Parser::_parse_column_predicate() {
    ColumnPredicate predicate;
    _parse_column(predicate.table_name, predicate.column_name);
    _parse_column_predicate_operator(predicate);
    return predicate;
}

void Parser::_parse_column(std::string &table_name, std::string &column_name) {
    auto name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    if (_scanner.lookahead() == TokenTag::DOT) {
        _scanner.match_token(TokenTag::DOT);
        table_name = name;
        name = _scanner.match_token(TokenTag::IDENTIFIER).raw;
    }
    column_name = name;
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
    auto cmd = _scanner.match_token(TokenTag::DELETE);
    _scanner.match_token(TokenTag::FROM);
    DeleteRecordActor actor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
    _parse_where_clause(actor.predicates);
    _scanner.match_token(TokenTag::SEMICOLON);
    for (auto &&pred : actor.predicates) {
        if (pred.cross_table) {
            throw ParserError{"Cross-table predicates are not allowed in DELETE commands.", cmd.offset};
        }
        if (pred.table_name.empty()) {
            pred.table_name = actor.table_name;
        } else if (pred.table_name != actor.table_name) {
            throw ParserError{
                std::string{"Irrelevant table \""}.append(pred.table_name).append("\" in column predicate."),
                cmd.offset};
        }
    }
    return actor;
}

Actor Parser::_parse_update_statement() {
    auto cmd = _scanner.match_token(TokenTag::UPDATE);
    UpdateRecordActor actor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
    _parse_set_clause(actor);
    _parse_where_clause(actor.predicates);
    for (auto &&pred : actor.predicates) {
        if (pred.cross_table) {
            throw ParserError{"Cross-table predicates are not allowed in UPDATE commands.", cmd.offset};
        }
        if (pred.table_name.empty()) {
            pred.table_name = actor.table_name;
        } else if (pred.table_name != actor.table_name) {
            throw ParserError{
                std::string{"Irrelevant table \""}.append(pred.table_name).append("\" in column predicate."),
                cmd.offset};
        }
    }
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

void Parser::_parse_set_clause(UpdateRecordActor &actor) {
    _scanner.match_token(TokenTag::SET);
    actor.columns.emplace_back(_scanner.match_token(TokenTag::IDENTIFIER).raw);
    _scanner.match_token(TokenTag::EQUAL);
    actor.lengths.emplace_back(_parse_value(actor.values));
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        actor.columns.emplace_back(_scanner.match_token(TokenTag::IDENTIFIER).raw);
        _scanner.match_token(TokenTag::EQUAL);
        actor.lengths.emplace_back(_parse_value(actor.values));
    }
}

Actor Parser::_parse_select_statement() {
    auto cmd = _scanner.match_token(TokenTag::SELECT);
    SelectRecordActor actor;
    _parse_selector(actor);
    _scanner.match_token(TokenTag::FROM);
    _parse_selection_table_list(actor.tables);
    _parse_where_clause(actor.predicates);
    _scanner.match_token(TokenTag::SEMICOLON);
    
    if (!actor.wildcard) {
        for (auto &&t : actor.selected_tables) {
            if (t.empty()) {
                if (actor.tables.size() == 1) {
                    t = actor.tables[0];
                } else {
                    throw ParserError{"Unspecific table name in multi-table query selector.", cmd.offset};
                }
            } else if (std::find(actor.tables.cbegin(), actor.tables.cend(), t) == actor.tables.cend()) {
                throw ParserError{std::string{"Irrelevant table \""}.append(t).append("\" in selector."), cmd.offset};
            }
        }
    }
    for (auto &&pred : actor.predicates) {
        auto &&t = pred.table_name;
        if (t.empty()) {
            if (actor.tables.size() == 1) {
                t = actor.tables[0];
            } else {
                throw ParserError{"Unspecific table name in multi-table query predicate.", cmd.offset};
            }
        } else if (std::find(actor.tables.cbegin(), actor.tables.cend(), t) == actor.tables.cend()) {
            throw ParserError{std::string{"Irrelevant table \""}.append(t).append("\" in predicate."), cmd.offset};
        }
        if (pred.cross_table) {
            auto &&rhs_tab = pred.rhs_table_name;
            if (rhs_tab.empty()) {
                if (actor.tables.size() == 1) {
                    rhs_tab = actor.tables[0];
                } else {
                    throw ParserError{"Unspecific table name in multi-table query predicate.", cmd.offset};
                }
            } else if (std::find(actor.tables.cbegin(), actor.tables.cend(), rhs_tab) == actor.tables.cend()) {
                throw ParserError{
                    std::string{"Irrelevant table \""}.append(rhs_tab).append("\" in predicate."),
                    cmd.offset};
            }
        }
    }
    
    if (actor.function != AggregateFunction::NONE &&
        actor.function != AggregateFunction::COUNT &&
        actor.wildcard) {
        throw ParserError{
            std::string{"Cannot apply aggregate function "}
                .append(AggregateFunctionHelper::name(actor.function))
                .append(" on multiple columns."), cmd.offset};
    }
    
    return actor;
}

void Parser::_parse_selector(SelectRecordActor &actor) {
    
    if (actor.wildcard) {
        throw ParserError{"Multiple selectors conflict with wildcard.", _scanner.current_offset()};
    }
    
    if (_scanner.lookahead() == TokenTag::WILDCARD) {
        _scanner.match_token(TokenTag::WILDCARD);
        actor.wildcard = true;
        return;
    }
    
    auto parse_aggregate_function = [&](auto func_token, auto func) {
        _scanner.match_token(func_token);
        actor.function = func;
        _scanner.match_token(TokenTag::LEFT_PARENTHESIS);
        if (_scanner.lookahead() == TokenTag::WILDCARD) {
            _scanner.match_token(TokenTag::WILDCARD);
            actor.wildcard = true;
        } else {
            actor.selected_tables.emplace_back();
            actor.selected_columns.emplace_back();
            _parse_column(actor.selected_tables.back(), actor.selected_columns.back());
        }
        _scanner.match_token(TokenTag::RIGHT_PARENTHESIS);
    };
    
    switch (_scanner.lookahead()) {
        case TokenTag::MIN:
            parse_aggregate_function(TokenTag::MIN, AggregateFunction::MIN);
            break;
        case TokenTag::MAX:
            parse_aggregate_function(TokenTag::MAX, AggregateFunction::MAX);
            break;
        case TokenTag::AVG:
            parse_aggregate_function(TokenTag::AVG, AggregateFunction::AVERAGE);
            break;
        case TokenTag::SUM:
            parse_aggregate_function(TokenTag::SUM, AggregateFunction::SUM);
            break;
        case TokenTag::COUNT:
            parse_aggregate_function(TokenTag::COUNT, AggregateFunction::COUNT);
            break;
        default:
            _parse_selected_columns(actor);
            break;
    }
}

void Parser::_parse_selected_columns(SelectRecordActor &actor) {
    actor.selected_tables.emplace_back();
    actor.selected_columns.emplace_back();
    _parse_column(actor.selected_tables.back(), actor.selected_columns.back());
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        actor.selected_tables.emplace_back();
        actor.selected_columns.emplace_back();
        _parse_column(actor.selected_tables.back(), actor.selected_columns.back());
    }
}

void Parser::_parse_selection_table_list(std::vector<std::string> &tables) {
    tables.emplace_back(_scanner.match_token(TokenTag::IDENTIFIER).raw);
    while (_scanner.lookahead() == TokenTag::COMMA) {
        _scanner.match_token(TokenTag::COMMA);
        auto table = _scanner.match_token(TokenTag::IDENTIFIER);
        
        // make sure that the list does not contain redundant tables.
        if (std::find(tables.cbegin(), tables.cend(), table.raw) != tables.cend()) {
            throw ParserError{"Redundant selection source table.", table.offset};
        }
        tables.emplace_back(table.raw);
    }
}

Actor Parser::_parse_exit_statement() {
    _scanner.match_token(TokenTag::EXIT);
    _scanner.match_token(TokenTag::SEMICOLON);
    return ExitActor{};
}

Actor Parser::_parse_execute_statement() {
    _scanner.match_token(TokenTag::SOURCE);
    auto file_name = ValueDecoder::decode_char(_parse_string());
    _scanner.match_token(TokenTag::SEMICOLON);
    return ExecuteFileActor{std::string{file_name}};
}

void Parser::_parse_column_predicate_operand(ColumnPredicate &pred) {
    if (_scanner.lookahead() == TokenTag::IDENTIFIER) {
        _parse_column(pred.rhs_table_name, pred.rhs_column_name);
        pred.cross_table = true;
    } else {
        _parse_value(pred.operand);
    }
}

Actor Parser::_parse_commit_statement() {
    _scanner.match_token(TokenTag::COMMIT);
    _scanner.match_token(TokenTag::SEMICOLON);
    return CommitActor{};
}
    
}
