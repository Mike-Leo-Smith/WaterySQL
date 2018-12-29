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
        Printer::print(std::cout, "INSERT INTO ", table_name, " VALUES\n");
        auto field_pos = 0ul;
        auto field_index = 0ul;
        bool first_row = true;
        auto count = 0;
        for (auto &&c : field_counts) {
            if (++count > 10) {
                Printer::print(std::cout, "    ... (", field_counts.size(), " rows totally)\n");
                break;
            }
            Printer::print(std::cout, "  (");
            bool first = true;
            for (auto i = field_index; i < field_index + c; i++) {
                if (!first) { Printer::print(std::cout, ", "); }
                first = false;
                auto l = field_sizes[i];
                Printer::print(
                    std::cout, (l == 0 ? "NULL" : std::string_view{buffer.data() + field_pos, l}),
                    "[", l, " bytes]");
                field_pos += l;
            }
            field_index += c;
            Printer::print(std::cout, ")\n");
        }
        QueryEngine::instance().insert_records(table_name, buffer, field_sizes, field_counts);
        Printer::println(std::cout, "Done.\n");
    }
    
};

}

#endif  // WATERYSQL_INSERT_RECORD_ACTOR_H
