//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_UPDATE_RECORD_ACTOR_H
#define WATERYSQL_UPDATE_RECORD_ACTOR_H

#include "../config/config.h"
#include "column_predicate.h"
#include "../utility/memory/string_view_copier.h"
#include "predicate_operator_helper.h"
#include "../utility/io/printer.h"

namespace watery {

struct UpdateRecordActor {
    
    std::string table_name;
    std::vector<std::string> columns;
    std::vector<Byte> values;
    std::vector<uint16_t> lengths;
    std::vector<ColumnPredicate> predicates;
    
    explicit UpdateRecordActor(std::string_view n)
        : table_name{n} {}
    
    void operator()() const {
        std::cout << "UPDATE " << table_name << " SET\n";
        auto p = 0ul;
        auto idx = 0;
        for (auto &&col: columns) {
            auto l = lengths[idx++];
            std::cout << "  " << col << " = "
                      << (l == 0 ? "NULL" : values.data() + p)
                      << "\n";
            p += l;
        }
        if (!predicates.empty()) {
            std::cout << "WHERE\n  ";
            bool first = true;
            for (auto &&pred : predicates) {
                if (!first) { std::cout << " AND\n  "; }
                first = false;
                if (!table_name.empty()) {
                    std::cout << table_name << ".";
                }
                std::cout << pred.column_name << " " << PredicateOperatorHelper::operator_symbol(pred.op);
                if (!pred.operand.empty()) {
                    std::cout << " " << pred.operand.data();
                }
            }
            std::cout << "\n";
        } else {
            std::cout << " ALL\n";
        }
        std::cout << std::endl;
    }
    
};

}

#endif  // WATERYSQL_UPDATE_RECORD_ACTOR_H
