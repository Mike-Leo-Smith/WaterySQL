//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_SHOW_TABLES_ACTOR_H
#define WATERYSQL_SHOW_TABLES_ACTOR_H

#include "../system_management/system_manager.h"

namespace watery {

struct ShowTablesActor {
    
    void operator()() const {
        for (auto &&t : SystemManager::instance().all_tables()) {
            std::cout << t << std::endl;
        }
    }
    
};

}

#endif  // WATERYSQL_SHOW_TABLES_ACTOR_H
