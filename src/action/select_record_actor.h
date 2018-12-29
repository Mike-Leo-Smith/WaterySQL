//
// Created by Mike Smith on 2018-12-16.
//

#ifndef WATERYSQL_SELECT_RECORD_ACTOR_H
#define WATERYSQL_SELECT_RECORD_ACTOR_H

#include <vector>
#include <array>

#include "../config/config.h"
#include "column_predicate.h"
#include "../utility/io/column_predicate_printer.h"

namespace watery {

struct SelectRecordActor {
    
    std::vector<std::string> selected_tables;
    std::vector<std::string> selected_columns;
    std::vector<std::string> tables;
    std::vector<ColumnPredicate> predicates;
    bool wildcard{false};
    
    void operator()() const {
        Printer::print(std::cout, "SELECT\n");
        if (wildcard) {
            Printer::print(std::cout, "    *\n");
        } else {
            for (auto i = 0; i < selected_tables.size(); i++) {
                Printer::print(std::cout, "    ");
                if (!selected_tables[i].empty()) {
                    Printer::print(std::cout, selected_tables[i], ".");
                }
                Printer::print(std::cout, selected_columns[i], "\n");
            }
        }
        Printer::print(std::cout, "FROM\n");
        for (auto &&t: tables) {
            Printer::print(std::cout, "    ", t, "\n");
        }
        if (!predicates.empty()) {
            Printer::print(std::cout, "WHERE\n");
            for (auto &&pred : predicates) {
                ColumnPredicatePrinter::print(std::cout, pred);
            }
        } else {
            Printer::print(std::cout, "ALL\n");
        }
        Printer::println(std::cout);
    }
    
};

}

#endif  // WATERYSQL_SELECT_RECORD_ACTOR_H
