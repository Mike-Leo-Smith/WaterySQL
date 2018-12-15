//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_COLUMN_PREDICATE_H
#define WATERYSQL_COLUMN_PREDICATE_H

#include <vector>

#include "../config/config.h"
#include "column_predicate_operator.h"

namespace watery {

struct ColumnPredicate {
    
    Identifier table_name{0};
    Identifier column_name{0};
    
    ColumnPredicateOperator op;
    std::vector<Byte> operand;
    
};

}

#endif  // WATERYSQL_COLUMN_PREDICATE_H
