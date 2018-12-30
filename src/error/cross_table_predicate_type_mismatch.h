//
// Created by Mike Smith on 2018-12-30.
//

#ifndef WATERYSQL_CROSS_TABLE_PREDICATE_TYPE_MISMATCH_H
#define WATERYSQL_CROSS_TABLE_PREDICATE_TYPE_MISMATCH_H

#include <string>
#include "error.h"

namespace watery {

struct CrossTablePredicateTypeMismatch : public Error {

    CrossTablePredicateTypeMismatch(const std::string &lhs_table, const std::string &lhs_column, const std::string &rhs_table, const std::string &rhs_column)
        : Error{
        "CrossTablePredicateTypeMismatch",
        std::string{"Data type of column \""}
            .append(lhs_column)
            .append("\" in table \"")
            .append(lhs_table)
            .append("\" mismatches with column \"")
            .append(rhs_column)
            .append("\" in table \"")
            .append(rhs_table)
            .append("\".")} {}

};

}

#endif  // WATERYSQL_CROSS_TABLE_PREDICATE_TYPE_MISMATCH_H
