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
    
    Identifier table_name{0};
    
    std::vector<Byte> buffer;
    std::vector<uint16_t> field_sizes;
    std::vector<uint16_t> field_counts;
    
    explicit InsertRecordActor(std::string_view t) noexcept {
        StringViewCopier::copy(t, table_name);
    }
    
    void operator()() const {
        Printer::println(std::cout, "INSERT INTO ", table_name.data(), " VALUES(", field_counts.size(), ")");
        auto field_pos = 0ul;
        auto field_index = 0ul;
        bool first_row = true;
        for (auto &&c : field_counts) {
            Printer::print(std::cout, "  (");
            bool first = true;
            for (auto i = field_index; i < field_index + c; i++) {
                if (!first) { Printer::print(std::cout, ", "); }
                first = false;
                auto l = field_sizes[i];
                Printer::print(std::cout, (l == 0 ? "NULL" : std::string_view{buffer.data() + field_pos, l}),  ":", l);
                field_pos += l;
            }
            field_index += c;
            Printer::print(std::cout, ")\n");
        }
        Printer::println(std::cout);
        Printer::println(std::cout, "Inserting...");
        QueryEngine::instance().insert_records(table_name.data(), buffer, field_sizes, field_counts);
        Printer::println(std::cout, "Done.");
    }
    
};

}

#endif  // WATERYSQL_INSERT_RECORD_ACTOR_H
