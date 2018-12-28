//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_PREDICATE_OPERATOR_H
#define WATERYSQL_PREDICATE_OPERATOR_H

namespace watery {

enum struct PredicateOperator {
    EQUAL, UNEQUAL,
    LESS, LESS_EQUAL,
    GREATER, GREATER_EQUAL,
    IS_NULL, NOT_NULL
};

}

#endif  // WATERYSQL_PREDICATE_OPERATOR_H
