//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_INSERT_RECORD_ACTOR_H
#define WATERYSQL_INSERT_RECORD_ACTOR_H

#include <vector>
#include <bitset>
#include <string_view>

#include "../config/config.h"

namespace watery {

struct InsertRecordActor {
    
    char table_name[MAX_IDENTIFIER_LENGTH + 1]{0};
    std::vector<char> buffer;
    std::vector<uint16_t> field_lengths;
    std::vector<uint16_t> field_counts;
    
    explicit InsertRecordActor(std::string_view t) noexcept {
        StringViewCopier::copy(t, table_name);
    }
    
    void operator()() const {
        std::cout << "INSERT INTO " << table_name << " VALUES(" << field_counts.size() << ")" << "\n";
        auto field_pos = 0ul;
        auto field_index = 0ul;
        bool first_row = true;
        for (auto &&c : field_counts) {
            std::cout << "  (";
            bool first = true;
            for (auto i = field_index; i < field_index + c; i++) {
                if (!first) { std::cout << ", "; }
                first = false;
                auto l = field_lengths[i];
                std::cout << (l == 0 ? "NULL" : std::string_view{buffer.data() + field_pos, l}) << ":" << l;
                field_pos += l;
            }
            field_index += c;
            std::cout << ")\n";
        }
        std::cout << std::endl;
    }
    
};

}

#endif  // WATERYSQL_INSERT_RECORD_ACTOR_H
