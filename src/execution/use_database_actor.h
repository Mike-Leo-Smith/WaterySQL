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
    
    Identifier name{0};
    
    explicit UseDatabaseActor(std::string_view n) noexcept {
        StringViewCopier::copy(n, name);
    }
    
    void operator()() const {
        Printer::println(std::cout, "USE DATABASE ", name.data(), ";");
        SystemManager::instance().use_database(name.data());
    }
    
};

}

#endif  // WATERYSQL_USE_DATABASE_ACTOR_H
