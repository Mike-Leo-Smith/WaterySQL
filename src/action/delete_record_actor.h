#include <utility>

//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_DELETE_RECORD_ACTOR_H
#define WATERYSQL_DELETE_RECORD_ACTOR_H

#include <vector>

#include "../config/config.h"
#include "column_predicate.h"
#include "predicate_operator_helper.h"
#include "../utility/memory/string_view_copier.h"

namespace watery {

struct DeleteRecordActor {
    
    std::string table_name;
    std::vector<ColumnPredicate> predicates;
    
    explicit DeleteRecordActor(std::string_view tab) noexcept
        : table_name{tab} {}
    
    void operator()() const {
        Printer::print(std::cout, "DELETE FROM ", table_name);
        if (!predicates.empty()) {
            Printer::print(std::cout, " WHERE\n");
            for (auto &&pred : predicates) {
                ColumnPredicatePrinter::print(std::cout, pred);
            }
        } else {
            Printer::print(std::cout, " ALL\n");
        }
        auto[ms, n] = timed_run([tn = table_name, &preds = predicates] {
            return QueryEngine::instance().delete_records(tn, preds);
        });
        Printer::println(std::cout, "Done in ", ms, "ms with ", n, " row", n > 1 ? "s" : "", " deleted.\n");
    }
    
};

}

#endif  // WATERYSQL_DELETE_RECORD_ACTOR_H
