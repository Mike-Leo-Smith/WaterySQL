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

    std::vector<std::array<Byte, MAX_FIELD_COUNT + 1>> selections;
    std::vector<std::array<Byte, MAX_FIELD_COUNT + 1>> tables;
    std::vector<ColumnPredicate> predicates;
    
    void operator()() const {
        std::cout << "SELECT\n  ";
        auto first = true;
        for (auto i = 0; i < selections.size(); i += 2) {
            if (!first) { std::cout << ",\n  "; }
            first = false;
            if (std::string_view s{selections[i].data()}; !s.empty()) {
                std::cout << s << ".";
            }
            std::cout << selections[i + 1].data();
        }
        std::cout << "\nFROM\n  ";
        first = true;
        for (auto &&t: tables) {
            if (!first) { std::cout << ",\n  "; }
            first = false;
            std::cout << t.data();
        }
        std::cout << "\n";
        if (!predicates.empty()) {
            std::cout << "WHERE\n  ";
            first = true;
            for (auto &&pred : predicates) {
                if (!first) { std::cout << " AND\n  "; }
                first = false;
                std::string_view table_name{pred.table_name};
                std::string_view column_name{pred.column_name};
                if (!table_name.empty()) {
                    std::cout << table_name << ".";
                }
                std::cout << column_name << " " << ColumnPredicateHelper::operator_symbol(pred.op);
                if (!pred.operand.empty()) {
                    std::cout << " " << std::string_view{pred.operand.data()};
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
