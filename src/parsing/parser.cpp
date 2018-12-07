//
// Created by Mike Smith on 2018-12-03.
//

#include "parser.h"
#include "../execution/show_databases_actor.h"
#include "../errors/parser_error.h"
#include "../execution/show_tables_actor.h"
#include "../execution/create_database_actor.h"
#include "../execution/use_database_actor.h"
#include "../execution/drop_database_actor.h"

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
    case TokenTag::DATABASE: {
        _scanner.match_token(TokenTag::DATABASE);
        auto identifier = _scanner.match_token(TokenTag::IDENTIFIER);
        actor = CreateDatabaseActor{identifier.raw};
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
    _scanner.match_token(TokenTag::DATABASE);
    UseDatabaseActor actor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

Actor Parser::_parse_drop_statement() {
    _scanner.match_token(TokenTag::DROP);
    Actor actor;
    switch (_scanner.lookahead()) {
    case TokenTag::DATABASE: {
        _scanner.match_token(TokenTag::DATABASE);
        actor = DropDatabaseActor{_scanner.match_token(TokenTag::IDENTIFIER).raw};
        break;
    }
    default: {
        auto token = _scanner.match_token(_scanner.lookahead());
        throw ParserError{
            std::string{"Unexpected token \""}.append(token.raw).append("\" after DROP."), token.offset};
    }
    }
    _scanner.match_token(TokenTag::SEMICOLON);
    return actor;
}

}
