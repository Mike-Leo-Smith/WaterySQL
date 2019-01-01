//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_CREATE_INDEX_ACTOR_H
#define WATERYSQL_CREATE_INDEX_ACTOR_H

#include <string>
#include <string_view>

#include "../system/system_manager.h"

namespace watery {

struct CreateIndexActor {
    
    std::string table_name;
    std::string column_name;
    
    CreateIndexActor(std::string_view tab, std::string_view col) noexcept
        : table_name{tab}, column_name{col} {}
    
    void operator()() const {
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::println(f, "CREATE INDEX ", table_name, "(", column_name, ")");
        }
        auto ms = timed_run([tn = table_name, cn = column_name] {
            SystemManager::instance().create_index(tn, cn);
        }).first;
        std::ofstream f{RESULT_FILE_NAME, std::ios::app};
        Printer::println(f, "Done in ", ms, "ms.<br/>");
    }
    
};

}

#endif  // WATERYSQL_CREATE_INDEX_ACTOR_H
