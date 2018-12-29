//
// Created by Mike Smith on 2018-12-29.
//

#ifndef WATERYSQL_COLUMN_PREDICATE_PRINTER_H
#define WATERYSQL_COLUMN_PREDICATE_PRINTER_H

#include "printer.h"
#include "../../action/column_predicate.h"
#include "../../action/predicate_operator_helper.h"

namespace watery {

struct ColumnPredicatePrinter : NonTrivialConstructible {
    
    template<typename Stream>
    static void print(Stream &os, const ColumnPredicate &pred) noexcept {
        Printer::print(std::cout, "    ");
        if (!pred.table_name.empty()) {
            Printer::print(os, pred.table_name, ".");
        }
        Printer::print(os, pred.column_name, " ", PredicateOperatorHelper::symbol(pred.op), " ");
        if (pred.op != PredicateOperator::IS_NULL && pred.op != PredicateOperator::NOT_NULL) {
            if (pred.cross_table) {
                if (!pred.rhs_table_name.empty()) {
                    Printer::print(os, pred.rhs_table_name, ".");
                }
                Printer::print(os, pred.rhs_column_name);
            } else {
                Printer::print(os, pred.operand.data());
            }
        }
        Printer::print(os, "\n");
    }
    
};

}

#endif  // WATERYSQL_COLUMN_PREDICATE_PRINTER_H
