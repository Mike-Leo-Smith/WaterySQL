//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_SINGLE_TABLE_PREDICATE_H
#define WATERYSQL_SINGLE_TABLE_PREDICATE_H

#include <string>
#include <memory>
#include <vector>

#include "../data/predicate_operator.h"
#include "../record/table.h"
#include "../utility/memory/value_decoder.h"
#include "query_plan.h"

namespace watery {

struct SingleTablePredicate {
    
    ColumnOffset column_offset;
    
    PredicateOperator op;
    std::vector<Byte> operand;
    
    QueryPlan query_plan;
    uint64_t cost;
    
    SingleTablePredicate(
        ColumnOffset cid, PredicateOperator op,
        std::vector<Byte> opr, QueryPlan qp, uint64_t c)
        : column_offset{cid}, op{op}, operand{std::move(opr)}, query_plan{qp}, cost{c} {}
    
    bool operator<(const SingleTablePredicate &rhs) const noexcept {
        return cost < rhs.cost;
    }
    
};

}

#endif  // WATERYSQL_SINGLE_TABLE_PREDICATE_H
