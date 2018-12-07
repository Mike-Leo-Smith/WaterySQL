//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_DROP_DATABASE_ACTOR_H
#define WATERYSQL_DROP_DATABASE_ACTOR_H

#include <string>
#include <string_view>

#include "../system_management/system_manager.h"

namespace watery {

struct DropDatabaseActor {
    
    const std::string name;
    
    explicit DropDatabaseActor(std::string_view name)
        : name{name} {}
    
    void operator()() const {
        SystemManager::instance().delete_database(name);
    }
    
};

}

#endif  // WATERYSQL_DROP_DATABASE_ACTOR_H
