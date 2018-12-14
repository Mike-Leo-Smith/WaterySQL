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
    
    char name[MAX_IDENTIFIER_LENGTH + 1]{};
    
    explicit UseDatabaseActor(std::string_view n) noexcept {
        n.copy(name, n.size());
    }
    
    void operator()() const {
        SystemManager::instance().use_database(name);
    }
    
};

}

#endif  // WATERYSQL_USE_DATABASE_ACTOR_H
