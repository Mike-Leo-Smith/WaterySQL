//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_COLUMN_PREDICATE_HELPER_H
#define WATERYSQL_COLUMN_PREDICATE_HELPER_H

#include <string_view>
#include "column_predicate_operator.h"
#include "column_predicate.h"

namespace watery {

struct ColumnPredicateHelper {
    
    static std::string_view operator_symbol(ColumnPredicateOperator op) noexcept {
        switch (op) {
            case ColumnPredicateOperator::EQUAL:
                return "=";
            case ColumnPredicateOperator::UNEQUAL:
                return "<>";
            case ColumnPredicateOperator::LESS:
                return "<";
            case ColumnPredicateOperator::LESS_EQUAL:
                return "<=";
            case ColumnPredicateOperator::GREATER:
                return ">";
            case ColumnPredicateOperator::GREATER_EQUAL:
                return ">=";
            case ColumnPredicateOperator::IS_NULL:
                return "IS NULL";
            case ColumnPredicateOperator::NOT_NULL:
                return "IS NOT NULL";
            default:
                return "UNKNOWN";
        }
    }
    
};

}

#endif  // WATERYSQL_COLUMN_PREDICATE_HELPER_H
