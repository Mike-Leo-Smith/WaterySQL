//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_INSERT_RECORD_ACTOR_H
#define WATERYSQL_INSERT_RECORD_ACTOR_H

#include <vector>
#include <bitset>
#include <string_view>

#include "../config/config.h"
#include "../utility/io/printer.h"
#include "../query/query_engine.h"

namespace watery {

struct InsertRecordActor {
    
    std::string table_name;
    
    std::vector<Byte> buffer;
    std::vector<uint16_t> field_sizes;
    std::vector<uint16_t> field_counts;
    
    explicit InsertRecordActor(std::string_view t) noexcept
        : table_name{t} {}
    
    void operator()() const {
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::print(f, "INSERT INTO ", table_name, " VALUES<br/>");
        }
        auto field_pos = 0ul;
        auto field_index = 0ul;
        bool first_row = true;
        auto count = 0;
    
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            {
                HtmlTablePrinter printer{f};
                for (auto &&c : field_counts) {
                    std::vector<std::string> row;
                
                    for (auto i = field_index; i < field_index + c; i++) {
                        auto l = field_sizes[i];
                        row.emplace_back(l == 0 ? "NULL" : std::string_view{buffer.data() + field_pos, l});
                        field_pos += l;
                    }
                    printer.print_row(row);
                    field_index += c;
                    if (++count > 10) { break; }
                }
            }
            if (count < field_counts.size()) {
                Printer::print(f, "    ... (", field_counts.size(), " rows totally)<br/>");
            }
        }
        auto[ms, n] = timed_run([tn = table_name, &buf = buffer, &fs = field_sizes, &fc = field_counts] {
            return QueryEngine::instance().insert_records(tn, buf, fs, fc);
        });
        
        std::ofstream f{RESULT_FILE_NAME, std::ios::app};
        Printer::println(f, "Done in ", ms, "ms with ", n, " row", n > 1 ? "s" : "", " inserted.<br/>");
    }
    
};

}

#endif  // WATERYSQL_INSERT_RECORD_ACTOR_H
