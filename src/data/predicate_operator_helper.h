//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_PREDICATE_OPERATOR_HELPER_H
#define WATERYSQL_PREDICATE_OPERATOR_HELPER_H

#include <string_view>
#include "predicate_operator.h"
#include "../action/column_predicate.h"

namespace watery {

struct PredicateOperatorHelper {
    
    static std::string_view symbol(PredicateOperator op) noexcept {
        switch (op) {
            case PredicateOperator::EQUAL:
                return "=";
            case PredicateOperator::UNEQUAL:
                return "&lt;&gt;";
            case PredicateOperator::LESS:
                return "&lt;";
            case PredicateOperator::LESS_EQUAL:
                return "&lt;=";
            case PredicateOperator::GREATER:
                return "&gt;";
            case PredicateOperator::GREATER_EQUAL:
                return "&gt;=";
            case PredicateOperator::IS_NULL:
                return "IS NULL";
            case PredicateOperator::NOT_NULL:
                return "IS NOT NULL";
            default:
                return "UNKNOWN";
        }
    }
    
    static PredicateOperator reversed(PredicateOperator op) noexcept {
        switch (op) {
            case PredicateOperator::LESS:
                return PredicateOperator::GREATER;
            case PredicateOperator::LESS_EQUAL:
                return PredicateOperator::GREATER_EQUAL;
            case PredicateOperator::GREATER:
                return PredicateOperator::LESS;
            case PredicateOperator::GREATER_EQUAL:
                return PredicateOperator::LESS_EQUAL;
            default:
                return op;
        }
    }
    
};

}

#endif  // WATERYSQL_PREDICATE_OPERATOR_HELPER_H
