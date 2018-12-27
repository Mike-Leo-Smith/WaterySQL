#include <utility>

//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_DELETE_RECORD_ACTOR_H
#define WATERYSQL_DELETE_RECORD_ACTOR_H

#include <vector>

#include "../config/config.h"
#include "column_predicate.h"
#include "column_predicate_helper.h"
#include "../utility/memory/string_view_copier.h"

namespace watery {

struct DeleteRecordActor {
    
    std::string table_name;
    mutable std::vector<ColumnPredicate> predicates;
    
    explicit DeleteRecordActor(std::string_view tab) noexcept
        : table_name{tab} {}
    
    void operator()() const {
        Printer::print(std::cout, "DELETE FROM ", table_name);
        if (!predicates.empty()) {
            Printer::print(std::cout, " WHERE\n  ");
            bool first = true;
            for (auto &&pred : predicates) {
                if (!first) { Printer::print(std::cout, " AND\n  "); }
                first = false;
                std::string_view table_name{pred.table_name};
                if (!table_name.empty()) {
                    Printer::print(std::cout, table_name, ".");
                }
                Printer::print(
                    std::cout, pred.column_name, " ",
                    ColumnPredicateHelper::operator_symbol(pred.op));
                if (!pred.operand.empty()) {
                    Printer::print(std::cout, " ", pred.operand.data());
                }
            }
            Printer::print(std::cout, "\n");
        } else {
            Printer::print(std::cout, " ALL\n");
        }
        Printer::println(std::cout);
        QueryEngine::instance().delete_record(table_name, predicates);
    }
    
};

}

#endif  // WATERYSQL_DELETE_RECORD_ACTOR_H
