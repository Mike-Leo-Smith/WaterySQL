#include <utility>

//
// Created by Mike Smith on 2018-12-30.
//

#ifndef WATERYSQL_CROSS_TABLE_PREDICATE_H
#define WATERYSQL_CROSS_TABLE_PREDICATE_H

#include <string>
#include <memory>

#include "../record/table.h"
#include "../data/predicate_operator.h"

namespace watery {

struct CrossTablePredicate {
    
    std::string lhs_table_name;
    ColumnOffset lhs_column_offset;
    
    std::string rhs_table_name;
    ColumnOffset rhs_column_offset;
    
    PredicateOperator op;
    
    CrossTablePredicate(
        std::string lhs_table_name, ColumnOffset lhs_column_offset,
        std::string rhs_table_name, ColumnOffset rhs_column_offset, PredicateOperator op)
        : lhs_table_name{std::move(lhs_table_name)}, lhs_column_offset{lhs_column_offset},
          rhs_table_name{std::move(rhs_table_name)}, rhs_column_offset{rhs_column_offset}, op(op) {}
    
};

}

#endif  // WATERYSQL_CROSS_TABLE_PREDICATE_H
