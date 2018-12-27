//
// Created by Mike Smith on 2018-12-16.
//

#ifndef WATERYSQL_SELECT_RECORD_ACTOR_H
#define WATERYSQL_SELECT_RECORD_ACTOR_H

#include <vector>
#include <array>

#include "../config/config.h"
#include "column_predicate.h"

namespace watery {

struct SelectRecordActor {

    std::vector<std::string> selections;
    std::vector<std::string> tables;
    std::vector<ColumnPredicate> predicates;
    
    void operator()() const {
        std::cout << "SELECT\n  ";
        auto first = true;
        for (auto i = 0; i < selections.size(); i += 2) {
            if (!first) { std::cout << ",\n  "; }
            first = false;
            if (!selections[i].empty()) {
                std::cout << selections[i] << ".";
            }
            std::cout << selections[i + 1];
        }
        std::cout << "\nFROM\n  ";
        first = true;
        for (auto &&t: tables) {
            if (!first) { std::cout << ",\n  "; }
            first = false;
            std::cout << t;
        }
        std::cout << "\n";
        if (!predicates.empty()) {
            std::cout << "WHERE\n  ";
            first = true;
            for (auto &&pred : predicates) {
                if (!first) { std::cout << " AND\n  "; }
                first = false;
                if (!pred.table_name.empty()) {
                    std::cout << pred.table_name << ".";
                }
                std::cout << pred.column_name << " " << ColumnPredicateHelper::operator_symbol(pred.op);
                if (!pred.operand.empty()) {
                    std::cout << " " << pred.operand.data();
                }
            }
            std::cout << "\n";
        } else {
            std::cout << " ALL\n";
        }
    }
    
};

}

#endif  // WATERYSQL_SELECT_RECORD_ACTOR_H
