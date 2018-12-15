//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_UPDATE_RECORD_ACTOR_H
#define WATERYSQL_UPDATE_RECORD_ACTOR_H

#include "../config/config.h"
#include "column_predicate.h"
#include "../utility/memory/string_view_copier.h"
#include "column_predicate_helper.h"

namespace watery {

struct UpdateRecordActor {
    
    char table_name[MAX_IDENTIFIER_LENGTH + 1]{0};
    std::vector<std::array<Byte, MAX_IDENTIFIER_LENGTH + 1>> columns;
    std::vector<Byte> values;
    std::vector<uint16_t> lengths;
    std::vector<ColumnPredicate> predicates;
    
    explicit UpdateRecordActor(std::string_view n) {
        StringViewCopier::copy(n, table_name);
    }
    
    void operator()() const {
        std::cout << "UPDATE " << table_name << " SET\n";
        auto p = 0ul;
        auto idx = 0;
        for (auto &&col: columns) {
            auto l = lengths[idx++];
            std::cout << "  " << col.data() << " = "
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
        std::cout << std::endl;
    }
    
};

}

#endif  // WATERYSQL_UPDATE_RECORD_ACTOR_H
