//
// Created by Mike Smith on 2018-12-29.
//

#ifndef WATERYSQL_COLUMN_PREDICATE_PRINTER_H
#define WATERYSQL_COLUMN_PREDICATE_PRINTER_H

#include "printer.h"
#include "../../action/column_predicate.h"
#include "../../data/predicate_operator_helper.h"

namespace watery {

struct ColumnPredicatePrinter : NonConstructible {
    
    template<typename Stream>
    static void print(Stream &os, const ColumnPredicate &pred) noexcept {
        Printer::print(os, "&nbsp;&nbsp;");
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
                Printer::print(os, std::string_view{pred.operand.data(), pred.operand.size()});
            }
        }
        Printer::print(os, "<br/>");
    }
    
};

}

#endif  // WATERYSQL_COLUMN_PREDICATE_PRINTER_H
