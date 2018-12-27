//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_PREDICATE_H
#define WATERYSQL_PREDICATE_H

#include <string>
#include <memory>
#include <vector>

#include "../action/column_predicate_operator.h"
#include "../record/table.h"

namespace watery {

struct predicate {
    
    std::shared_ptr<Table> table;
    ColumnOffset column_offset;
    
    ColumnPredicateOperator op;
    std::vector<Byte> operand;

};

}

#endif  // WATERYSQL_PREDICATE_H
