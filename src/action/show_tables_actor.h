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
        auto[ms, list] = timed_run([] {
            return SystemManager::instance().table_list();
        });
        
        std::ofstream f{RESULT_FILE_NAME};
        {
            HtmlTablePrinter p{f};
            p.print_header({"Tables"});
            for (auto &&t : list) {
                p.print_row({t});
            }
        }
        Printer::println(f, "Done in ", ms, "ms.\n");
    }
    
};

}

#endif  // WATERYSQL_SHOW_TABLES_ACTOR_H
