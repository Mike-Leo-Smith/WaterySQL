#include <utility>

//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_DELETE_RECORD_ACTOR_H
#define WATERYSQL_DELETE_RECORD_ACTOR_H

#include <vector>

#include "../config/config.h"
#include "column_predicate.h"
#include "../data/predicate_operator_helper.h"
#include "../utility/memory/string_view_copier.h"

namespace watery {

struct DeleteRecordActor {
    
    std::string table_name;
    std::vector<ColumnPredicate> predicates;
    
    explicit DeleteRecordActor(std::string_view tab) noexcept
        : table_name{tab} {}
    
    void operator()() const {
    
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::print(f, "DELETE FROM ", table_name);
            if (!predicates.empty()) {
                Printer::print(f, " WHERE<br/>");
                for (auto &&pred : predicates) {
                    ColumnPredicatePrinter::print(f, pred);
                }
            } else {
                Printer::print(f, " ALL<br/>");
            }
        }
        auto[ms, n] = timed_run([tn = table_name, &preds = predicates] {
            return QueryEngine::instance().delete_records(tn, preds);
        });
        
        std::ofstream f{RESULT_FILE_NAME, std::ios::app};
        Printer::println(f, "Done in ", ms, "ms with ", n, " row", n > 1 ? "s" : "", " deleted.<br/>");
    }
    
};

}

#endif  // WATERYSQL_DELETE_RECORD_ACTOR_H
