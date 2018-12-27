//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_COLUMN_PREDICATE_OPERATOR_H
#define WATERYSQL_COLUMN_PREDICATE_OPERATOR_H

namespace watery {

enum struct ColumnPredicateOperator {
    EQUAL, UNEQUAL,
    LESS, LESS_EQUAL,
    GREATER, GREATER_EQUAL,
    IS_NULL, NOT_NULL
};

}

#endif  // WATERYSQL_COLUMN_PREDICATE_OPERATOR_H