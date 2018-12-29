//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_SHOW_TABLES_ACTOR_H
#define WATERYSQL_SHOW_TABLES_ACTOR_H

#include "../system/system_manager.h"
#include "../utility/io/printer.h"

namespace watery {

struct ShowTablesActor {
    
    void operator()() const {
        Printer::println(std::cout, "SHOW TABLES");
        for (auto &&t : SystemManager::instance().table_list()) {
            Printer::print(std::cout, "    ", t, "\n");
        }
        Printer::println(std::cout, "Done.");
    }
    
};

}

#endif  // WATERYSQL_SHOW_TABLES_ACTOR_H
