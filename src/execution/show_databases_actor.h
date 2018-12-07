//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_SHOW_DATABASES_ACTOR_H
#define WATERYSQL_SHOW_DATABASES_ACTOR_H

#include "../system_management/system_manager.h"

namespace watery {

struct ShowDatabasesActor {
    
    void operator()() const {
        for (auto &&db : SystemManager::instance().all_databases()) {
            std::cout << db << std::endl;
        }
    }
    
};

}

#endif  // WATERYSQL_SHOW_DATABASES_ACTOR_H