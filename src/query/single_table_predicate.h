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

namespace watery {

struct SingleTablePredicate {
    
    std::shared_ptr<Table> table;
    ColumnOffset column_offset;
    
    PredicateOperator op;
    std::vector<Byte> operand;
    
    uint64_t estimated_cost;
    
    SingleTablePredicate(std::shared_ptr<Table> t, ColumnOffset cid, PredicateOperator op, std::vector<Byte> opr, uint64_t c)
        : table{std::move(t)}, column_offset{cid}, op{op}, estimated_cost{c} {
        if (op != PredicateOperator::IS_NULL && op != PredicateOperator::NOT_NULL) {
            auto &&data_desc = table->descriptor().field_descriptors[column_offset].data_descriptor;
            operand.resize(data_desc.length);
            ValueDecoder::decode(data_desc.type, opr.data(), operand.data());
        }
    }
    
    bool operator<(const SingleTablePredicate &rhs) const noexcept {
        return estimated_cost < rhs.estimated_cost;
    }
    
};

}

#endif  // WATERYSQL_SINGLE_TABLE_PREDICATE_H
