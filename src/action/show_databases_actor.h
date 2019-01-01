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
        auto[ms, list] = timed_run([] {
            return SystemManager::instance().database_list();
        });
        
        std::ofstream f{RESULT_FILE_NAME};
        {
            HtmlTablePrinter p{f};
            p.print_header({"Databases"});
            for (auto &&db : list) {
                p.print_row({db});
            }
        }
        Printer::println(f, "Done in ", ms, "ms.\n");
    }
    
};

}

#endif  // WATERYSQL_SHOW_DATABASES_ACTOR_H
