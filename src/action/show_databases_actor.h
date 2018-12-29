//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_SHOW_DATABASES_ACTOR_H
#define WATERYSQL_SHOW_DATABASES_ACTOR_H

#include "../system/system_manager.h"
#include "../utility/io/printer.h"

namespace watery {

struct ShowDatabasesActor {
    
    void operator()() const {
        Printer::println(std::cout, "SHOW DATABASES");
        for (auto &&db : SystemManager::instance().database_list()) {
            Printer::print(std::cout, "    ", db, "\n");
        }
        Printer::println(std::cout, "Done.");
    }
    
};

}

#endif  // WATERYSQL_SHOW_DATABASES_ACTOR_H
