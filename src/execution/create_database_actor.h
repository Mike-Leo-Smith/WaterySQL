//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_CREATE_DATABASE_ACTOR_H
#define WATERYSQL_CREATE_DATABASE_ACTOR_H

#include <string>
#include <string_view>
#include "../system_management/system_manager.h"

namespace watery {

struct CreateDatabaseActor {
    
    const std::string name;
    
    explicit CreateDatabaseActor(std::string_view name)
        : name{name} {}
    
    void operator()() const {
        SystemManager::instance().create_database(name);
    }
};

}

#endif  // WATERYSQL_CREATE_DATABASE_ACTOR_H
