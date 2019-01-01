//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_UPDATE_RECORD_ACTOR_H
#define WATERYSQL_UPDATE_RECORD_ACTOR_H

#include "../config/config.h"
#include "column_predicate.h"
#include "../utility/memory/string_view_copier.h"
#include "../data/predicate_operator_helper.h"
#include "../utility/io/printer.h"
#include "../utility/io/column_predicate_printer.h"

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
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::print(f, "UPDATE ", table_name, " SET<br/>");
            auto p = 0ul;
            auto idx = 0;
            for (auto &&col: columns) {
                auto l = lengths[idx++];
                Printer::print(
                    f, "&nbsp;&nbsp;", col, " = ", (l == 0 ? "NULL" : std::string_view{values.data() + p, l}), "<br/>");
                p += l;
            }
            if (!predicates.empty()) {
                Printer::print(f, "WHERE<br/>");
                for (auto &&pred : predicates) {
                    ColumnPredicatePrinter::print(f, pred);
                }
            } else {
                Printer::print(f, "ALL<br/>");
            }
        }
        auto[ms, n] = timed_run([t = table_name, &c = columns, &v = values, &l = lengths, &p = predicates] {
            return QueryEngine::instance().update_records(t, c, v, l, p);
        });
        
        std::ofstream f{RESULT_FILE_NAME, std::ios::app};
        Printer::println(f, "Done in ", ms, "ms with ", n, " row", n > 1 ? "s" : "", " updated.<br/>");
    }
    
};

}

#endif  // WATERYSQL_UPDATE_RECORD_ACTOR_H
