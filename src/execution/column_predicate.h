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
    
    char table_name[MAX_IDENTIFIER_LENGTH + 1]{0};
    char column_name[MAX_IDENTIFIER_LENGTH + 1]{0};
    
    ColumnPredicateOperator op;
    std::vector<Byte> operand;
    
};

}

#endif  // WATERYSQL_COLUMN_PREDICATE_H
