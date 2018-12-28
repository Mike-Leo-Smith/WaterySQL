//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_PREDICATE_OPERATOR_HELPER_H
#define WATERYSQL_PREDICATE_OPERATOR_HELPER_H

#include <string_view>
#include "predicate_operator.h"
#include "column_predicate.h"

namespace watery {

struct PredicateOperatorHelper {
    
    static std::string_view operator_symbol(PredicateOperator op) noexcept {
        switch (op) {
            case PredicateOperator::EQUAL:
                return "=";
            case PredicateOperator::UNEQUAL:
                return "<>";
            case PredicateOperator::LESS:
                return "<";
            case PredicateOperator::LESS_EQUAL:
                return "<=";
            case PredicateOperator::GREATER:
                return ">";
            case PredicateOperator::GREATER_EQUAL:
                return ">=";
            case PredicateOperator::IS_NULL:
                return "IS NULL";
            case PredicateOperator::NOT_NULL:
                return "IS NOT NULL";
            default:
                return "UNKNOWN";
        }
    }
    
};

}

#endif  // WATERYSQL_PREDICATE_OPERATOR_HELPER_H
