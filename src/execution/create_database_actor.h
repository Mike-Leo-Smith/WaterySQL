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
    
    char name[MAX_IDENTIFIER_LENGTH + 1]{0};
    
    explicit CreateDatabaseActor(std::string_view n) noexcept {
        StringViewCopier::copy(n, name);
    }
    
    void operator()() const {
        SystemManager::instance().create_database(name);
    }
};

}

#endif  // WATERYSQL_CREATE_DATABASE_ACTOR_H
