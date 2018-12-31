//
// Created by Mike Smith on 2018-12-16.
//

#ifndef WATERYSQL_SELECT_RECORD_ACTOR_H
#define WATERYSQL_SELECT_RECORD_ACTOR_H

#include <vector>
#include <array>
#include <iomanip>

#include "../config/config.h"
#include "column_predicate.h"
#include "../utility/io/column_predicate_printer.h"
#include "../data/aggregate_function.h"
#include "../data/aggregate_function_helper.h"

namespace watery {

struct SelectRecordActor {
    
    std::vector<std::string> selected_tables;
    std::vector<std::string> selected_columns;
    std::vector<std::string> tables;
    std::vector<ColumnPredicate> predicates;
    AggregateFunction function{AggregateFunction::NONE};
    bool wildcard{false};
    
    void operator()() const {
        Printer::print(std::cout, "SELECT\n");
        if (wildcard) {
            if (function == AggregateFunction::NONE) {
                Printer::print(std::cout, "  *\n");
            } else {
                Printer::print(std::cout, "  ", AggregateFunctionHelper::name(function), "(*)\n");
            }
        } else {
            if (function == AggregateFunction::NONE) {
                for (auto i = 0; i < selected_tables.size(); i++) {
                    Printer::print(std::cout, "  ", selected_tables[i], ".", selected_columns[i], "\n");
                }
            } else {
                Printer::print(
                    std::cout, "  ", AggregateFunctionHelper::name(function),
                    "(", selected_tables[0], ".", selected_columns[0], ")\n");
            }
        }
        Printer::print(std::cout, "FROM\n");
        for (auto &&t: tables) { Printer::print(std::cout, "  ", t, "\n"); }
        if (!predicates.empty()) {
            Printer::print(std::cout, "WHERE\n");
            for (auto &&pred : predicates) { ColumnPredicatePrinter::print(std::cout, pred); }
        } else {
            Printer::print(std::cout, "ALL\n");
        }
        
        auto accum = 0.0;
        auto[ms, n] = timed_run(
            [&st = selected_tables, &sc = selected_columns, &t = tables, &p = predicates, f = function, &a = accum] {
                auto first = true;
                return QueryEngine::instance().select_records(
                    st, sc, t, p, [&a, f, &first](const std::vector<std::string> &row) {
                        switch (f) {
                            case AggregateFunction::SUM:
                            case AggregateFunction::AVERAGE:
                                a += ValueDecoder::decode_double(row[0]);
                                break;
                            case AggregateFunction::MIN: {
                                auto x = ValueDecoder::decode_double(row[0]);
                                a = first ? x : std::min(a, x);
                                break;
                            }
                            case AggregateFunction::MAX: {
                                auto x = ValueDecoder::decode_double(row[0]);
                                a = first ? x : std::max(a, x);
                                break;
                            }
                            case AggregateFunction::NONE:
                                Printer::print(std::cout, "  | ");
                                for (auto &&s : row) {
                                    Printer::print(std::cout, s, " | ");
                                }
                                Printer::print(std::cout, "\n");
                                break;
                            default:
                                break;
                        }
                        first = false;
                    });
            });
        
        switch (function) {
            case AggregateFunction::AVERAGE:
                accum /= n;
                break;
            case AggregateFunction::COUNT:
                accum = n;
                break;
            default:
                break;
        }
        
        if (function != AggregateFunction::NONE) {
            n = 1;
            auto as_int = static_cast<int64_t>(accum);
            if (as_int == accum) {
                Printer::print(std::cout, "  result = ", as_int, "\n");
            } else {
                Printer::print(
                    std::cout, "  result = ",
                    std::setprecision(std::numeric_limits<double>::max_digits10 - 1),
                    accum, "\n");
            }
        }
        Printer::println(std::cout, "Done in ", ms, "ms with ", n, " row", n > 1 ? "s" : "", " selected.\n");
    }
    
};

}

#endif  // WATERYSQL_SELECT_RECORD_ACTOR_H
