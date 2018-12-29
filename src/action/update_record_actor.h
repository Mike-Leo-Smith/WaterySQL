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
        Printer::print(std::cout, "UPDATE ", table_name, " SET\n");
        auto p = 0ul;
        auto idx = 0;
        for (auto &&col: columns) {
            auto l = lengths[idx++];
            Printer::print(
                std::cout, "  ", col, " = ",
                (l == 0 ? "NULL" : std::string_view{values.data() + p, l}), "\n");
            p += l;
        }
        if (!predicates.empty()) {
            Printer::print(std::cout, "WHERE\n");
            for (auto &&pred : predicates) {
                ColumnPredicatePrinter::print(std::cout, pred);
            }
        } else {
            Printer::print(std::cout, "ALL\n");
        }
        QueryEngine::instance().update_records(table_name, columns, values, lengths, predicates);
        Printer::println(std::cout, "Done.\n");
    }
    
};

}

#endif  // WATERYSQL_UPDATE_RECORD_ACTOR_H
