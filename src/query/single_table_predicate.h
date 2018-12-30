//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_SINGLE_TABLE_PREDICATE_H
#define WATERYSQL_SINGLE_TABLE_PREDICATE_H

#include <string>
#include <memory>
#include <vector>

#include "../action/predicate_operator.h"
#include "../record/table.h"
#include "../utility/memory/value_decoder.h"
#include "query_plan.h"

namespace watery {

struct SingleTablePredicate {
    
    ColumnOffset column_offset;
    
    PredicateOperator op;
    std::vector<Byte> operand;
    
    QueryPlan query_plan;
    uint64_t estimated_cost;
    
    SingleTablePredicate(
        ColumnOffset cid, PredicateOperator op,
        std::vector<Byte> opr, DataDescriptor desc, QueryPlan qp, uint64_t c)
        : column_offset{cid}, op{op}, query_plan{qp}, estimated_cost{c} {
        if (op != PredicateOperator::IS_NULL && op != PredicateOperator::NOT_NULL) {
            operand.resize(desc.length);
            ValueDecoder::decode(desc.type, opr.data(), operand.data());
        }
    }
    
    bool operator<(const SingleTablePredicate &rhs) const noexcept {
        return estimated_cost < rhs.estimated_cost;
    }
    
};

}

#endif  // WATERYSQL_SINGLE_TABLE_PREDICATE_H
