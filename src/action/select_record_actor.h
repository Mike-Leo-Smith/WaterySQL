//
// Created by Mike Smith on 2018-12-16.
//

#ifndef WATERYSQL_SELECT_RECORD_ACTOR_H
#define WATERYSQL_SELECT_RECORD_ACTOR_H

#include <vector>
#include <array>
#include <iomanip>
#include <sstream>

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
        
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::print(f, "SELECT<br/>");
            if (wildcard) {
                if (function == AggregateFunction::NONE) {
                    Printer::print(f, "&nbsp;&nbsp;*<br/>");
                } else {
                    Printer::print(f, "&nbsp;&nbsp;", AggregateFunctionHelper::name(function), "(*)<br/>");
                }
            } else {
                if (function == AggregateFunction::NONE) {
                    for (auto i = 0; i < selected_tables.size(); i++) {
                        Printer::print(f, "&nbsp;&nbsp;", selected_tables[i], ".", selected_columns[i], "<br/>");
                    }
                } else {
                    Printer::print(
                        f, "&nbsp;&nbsp;", AggregateFunctionHelper::name(function),
                        "(", selected_tables[0], ".", selected_columns[0], ")<br/>");
                }
            }
            Printer::print(f, "FROM<br/>");
            for (auto &&t: tables) { Printer::print(f, "&nbsp;&nbsp;", t, "<br/>"); }
            if (!predicates.empty()) {
                Printer::print(f, "WHERE<br/>");
                for (auto &&pred : predicates) { ColumnPredicatePrinter::print(f, pred); }
            } else {
                Printer::print(f, "ALL<br/>");
            }
            Printer::println(f);
        }
        
        double ms = 0.0;
        size_t n = 0;
        {
            std::ofstream result_file{RESULT_FILE_NAME, std::ios::app};
            HtmlTablePrinter printer{result_file};
            
            if (function != AggregateFunction::NONE) {
                if (function != AggregateFunction::COUNT || !wildcard) {
                    printer.print_header({std::string{AggregateFunctionHelper::name(function)}.append("(").
                        append(selected_tables[0]).append(".").append(selected_columns[0]).append(")")});
                } else {
                    printer.print_header({std::string{AggregateFunctionHelper::name(function)}});
                }
            } else if (!wildcard) {
                std::vector<std::string> header;
                for (auto i = 0; i < selected_columns.size(); i++) {
                    header.emplace_back((selected_tables[i] + ".").append(selected_columns[i]));
                }
                printer.print_header(header);
            }
            
            auto accum = 0.0;
            std::tie(ms, n) = timed_run(
                [&st = selected_tables, &sc = selected_columns, &t = tables,
                    &p = predicates, f = function, &a = accum, &result_file, &printer] {
                    auto first = true;
                    return QueryEngine::instance().select_records(
                        st, sc, t, p, [&a, f, &first, &result_file, &printer](const std::vector<std::string> &row) {
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
                                    printer.print_row(row);
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
                    printer.print_row({std::to_string(as_int)});
                } else {
                    printer.print_row(
                        {(std::stringstream{}
                            << std::setprecision(std::numeric_limits<double>::max_digits10 - 1)
                            << accum).str()});
                }
            }
        }
        
        std::ofstream result_file{RESULT_FILE_NAME, std::ios::app};
        Printer::println(result_file, "Done in ", ms, "ms with ", n, " row", n > 1 ? "s" : "", " selected.<br/>");
    }
    
};

}

#endif  // WATERYSQL_SELECT_RECORD_ACTOR_H
