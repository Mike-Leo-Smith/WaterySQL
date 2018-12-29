//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_USE_DATABASE_ACTOR_H
#define WATERYSQL_USE_DATABASE_ACTOR_H

#include <string>
#include <string_view>
#include "../system/system_manager.h"

namespace watery {

struct UseDatabaseActor {
    
    std::string name;
    
    explicit UseDatabaseActor(std::string_view n) noexcept
        : name{n} {}
    
    void operator()() const {
        Printer::println(std::cout, "USE DATABASE ", name);
        auto ms = timed_run([name = name] { SystemManager::instance().use_database(name); }).first;
        Printer::println(std::cout, "Done in ", ms, "ms.\n");
    }
    
};

}

#endif  // WATERYSQL_USE_DATABASE_ACTOR_H
