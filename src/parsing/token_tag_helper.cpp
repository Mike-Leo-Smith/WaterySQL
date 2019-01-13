//
// Created by Mike Smith on 2018-12-13.
//

#include "token_tag_helper.h"

namespace watery {

const std::unordered_map<std::string_view, TokenTag> &TokenTagHelper::keyword_dict() noexcept {
    static std::unordered_map<std::string_view, TokenTag> dict{
        {"DATABASE",   TokenTag::DATABASE},
        {"DATABASES",  TokenTag::DATABASES},
        {"TABLE",      TokenTag::TABLE},
        {"TABLES",     TokenTag::TABLES},
        {"SHOW",       TokenTag::SHOW},
        {"CREATE",     TokenTag::CREATE},
        {"DROP",       TokenTag::DROP},
        {"USE",        TokenTag::USE},
        {"PRIMARY",    TokenTag::PRIMARY},
        {"KEY",        TokenTag::KEY},
        {"NOT",        TokenTag::NOT},
        {"NULL",       TokenTag::NUL},
        {"INSERT",     TokenTag::INSERT},
        {"INTO",       TokenTag::INTO},
        {"VALUES",     TokenTag::VALUES},
        {"DELETE",     TokenTag::DELETE},
        {"FROM",       TokenTag::FROM},
        {"WHERE",      TokenTag::WHERE},
        {"UPDATE",     TokenTag::UPDATE},
        {"SET",        TokenTag::SET},
        {"SELECT",     TokenTag::SELECT},
        {"IS",         TokenTag::IS},
        {"INT",        TokenTag::INT},
        {"INTEGER",    TokenTag::INT},
        {"CHAR",       TokenTag::CHAR},
        {"VARCHAR",    TokenTag::CHAR},
        {"DESC",       TokenTag::DESCRIBE},
        {"DESCRIBE",   TokenTag::DESCRIBE},
        {"REFERENCES", TokenTag::REFERENCES},
        {"INDEX",      TokenTag::INDEX},
        {"AND",        TokenTag::AND},
        {"DATE",       TokenTag::DATE},
        {"FLOAT",      TokenTag::FLOAT},
        {"REAL",       TokenTag::FLOAT},
        {"FOREIGN",    TokenTag::FOREIGN},
        {"UNIQUE",     TokenTag::UNIQUE},
        {"EXIT",       TokenTag::EXIT},
        {"QUIT",       TokenTag::EXIT},
        {"SOURCE",     TokenTag::SOURCE},
        {"MIN",        TokenTag::MIN},
        {"MAX",        TokenTag::MAX},
        {"AVG",        TokenTag::AVG},
        {"SUM",        TokenTag::SUM},
        {"COUNT",      TokenTag::COUNT},
        {"COMMIT",     TokenTag::COMMIT}
    };
    return dict;
}

std::string_view TokenTagHelper::name(TokenTag tag) noexcept {
    switch (tag) {
        case TokenTag::DATABASE:
            return "DATABASE";
        case TokenTag::DATABASES:
            return "DATABASES";
        case TokenTag::TABLE:
            return "TABLE";
        case TokenTag::TABLES:
            return "TABLES";
        case TokenTag::SHOW:
            return "SHOW";
        case TokenTag::CREATE:
            return "CREATE";
        case TokenTag::DROP:
            return "DROP";
        case TokenTag::USE:
            return "USE";
        case TokenTag::PRIMARY:
            return "PRIMARY";
        case TokenTag::KEY:
            return "KEY";
        case TokenTag::NOT:
            return "NOT";
        case TokenTag::NUL:
            return "NULL";
        case TokenTag::INSERT:
            return "INSERT";
        case TokenTag::INTO:
            return "INTO";
        case TokenTag::VALUES:
            return "VALUES";
        case TokenTag::DELETE:
            return "DELETE";
        case TokenTag::FROM:
            return "FROM";
        case TokenTag::WHERE:
            return "WHERE";
        case TokenTag::UPDATE:
            return "UPDATE";
        case TokenTag::SET:
            return "SET";
        case TokenTag::SELECT:
            return "SELECT";
        case TokenTag::IS:
            return "IS";
        case TokenTag::INT:
            return "INT";
        case TokenTag::UNIQUE:
            return "UNIQUE";
        case TokenTag::CHAR:
            return "CHAR";
        case TokenTag::DESCRIBE:
            return "DESCRIBE";
        case TokenTag::REFERENCES:
            return "REFERENCES";
        case TokenTag::INDEX:
            return "INDEX";
        case TokenTag::AND:
            return "AND";
        case TokenTag::DATE:
            return "DATE";
        case TokenTag::FLOAT:
            return "FLOAT";
        case TokenTag::FOREIGN:
            return "FOREIGN";
        case TokenTag::IDENTIFIER:
            return "IDENTIFIER";
        case TokenTag::NUMBER:
            return "NUMBER";
        case TokenTag::STRING:
            return "STRING";
        case TokenTag::END:
            return "<END>";
        case TokenTag::SEMICOLON:
            return ";";
        case TokenTag::COMMA:
            return ",";
        case TokenTag::LEFT_PARENTHESIS:
            return "(";
        case TokenTag::RIGHT_PARENTHESIS:
            return ")";
        case TokenTag::DOT:
            return ".";
        case TokenTag::EQUAL:
            return "=";
        case TokenTag::UNEQUAL:
            return "<>";
        case TokenTag::LESS:
            return "<";
        case TokenTag::LESS_EQUAL:
            return "<=";
        case TokenTag::GREATER:
            return ">";
        case TokenTag::GREATER_EQUAL:
            return ">=";
        case TokenTag::WILDCARD:
            return "*";
        case TokenTag::EXIT:
            return "EXIT";
        case TokenTag::SOURCE:
            return "SOURCE";
        case TokenTag::MIN:
            return "MIN";
        case TokenTag::MAX:
            return "MAX";
        case TokenTag::AVG:
            return "AVG";
        case TokenTag::SUM:
            return "SUM";
        case TokenTag::COUNT:
            return "COUNT";
        case TokenTag::COMMIT:
            return "COMMIT";
        default:
            return "UNKNOWN";
    }
}

}