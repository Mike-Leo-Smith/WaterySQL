//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_USE_DATABASE_ACTOR_H
#define WATERYSQL_USE_DATABASE_ACTOR_H

#include <string>
#include <string_view>
#include "../system_management/system_manager.h"

namespace watery {

struct UseDatabaseActor {
    
    const std::string name;
    
    explicit UseDatabaseActor(std::string_view name)
        : name{name} {}
    
    void operator()() const {
        SystemManager::instance().use_database(name);
    }
    
};

}

#endif  // WATERYSQL_USE_DATABASE_ACTOR_H
