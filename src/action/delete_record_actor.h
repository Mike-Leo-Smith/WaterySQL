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
    
    Identifier table_name{0};
    std::vector<ColumnPredicate> predicates;
    
    explicit DeleteRecordActor(std::string_view tab) noexcept {
        StringViewCopier::copy(tab, table_name);
    }
    
    void operator()() const {
        std::cout << "DELETE FROM " << table_name.data();
        if (!predicates.empty()) {
            std::cout << " WHERE\n  ";
            bool first = true;
            for (auto &&pred : predicates) {
                if (!first) { std::cout << " AND\n  "; }
                first = false;
                std::string_view table_name{pred.table_name.data()};
                if (!table_name.empty()) {
                    std::cout << table_name << ".";
                }
                std::cout << pred.column_name.data() << " " << ColumnPredicateHelper::operator_symbol(pred.op);
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

#endif  // WATERYSQL_DELETE_RECORD_ACTOR_H
