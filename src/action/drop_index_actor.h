//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DROP_INDEX_ACTOR_H
#define WATERYSQL_DROP_INDEX_ACTOR_H

#include <string>
#include <string_view>

#include "../utility/io/printer.h"
#include "../system/system_manager.h"

namespace watery {

struct DropIndexActor {
    
    std::string table_name;
    std::string column_name;
    
    DropIndexActor(std::string_view tab, std::string_view col) noexcept
        : table_name{tab}, column_name{tab} {}
    
    void operator()() const {
        Printer::println(std::cout, "DROP INDEX ", table_name, "(", column_name, ")");
        auto ms = timed_run([tn = table_name, cn = column_name] {
            SystemManager::instance().drop_index(tn, cn);
        }).first;
        Printer::println(std::cout, "Done in ", ms, "ms.\n");
    }
    
};

}

#endif  // WATERYSQL_DROP_INDEX_ACTOR_H
