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
    
    char name[MAX_IDENTIFIER_LENGTH + 1]{0};
    
    explicit DropDatabaseActor(std::string_view n) noexcept {
        StringViewCopier::copy(n, name);
    }
    
    void operator()() const {
        SystemManager::instance().delete_database(name);
    }
    
};

}

#endif  // WATERYSQL_DROP_DATABASE_ACTOR_H
