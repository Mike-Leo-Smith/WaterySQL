//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_COLUMN_PREDICATE_H
#define WATERYSQL_COLUMN_PREDICATE_H

#include <vector>
#include <string>

#include "../config/config.h"
#include "../data/predicate_operator.h"

namespace watery {

struct ColumnPredicate {
    
    std::string table_name;
    std::string column_name;
    
    PredicateOperator op;
    std::vector<Byte> operand;
    
    bool cross_table{false};
    std::string rhs_table_name;
    std::string rhs_column_name;
    
};

}

#endif  // WATERYSQL_COLUMN_PREDICATE_H
