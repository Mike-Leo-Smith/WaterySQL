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
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::println(f, "USE DATABASE ", name);
        }
        auto ms = timed_run([name = name] { SystemManager::instance().use_database(name); }).first;
        
        std::ofstream f{RESULT_FILE_NAME, std::ios::app};
        Printer::println(f, "Done in ", ms, "ms.<br/>");
    }
    
};

}

#endif  // WATERYSQL_USE_DATABASE_ACTOR_H
