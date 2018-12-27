//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_DROP_DATABASE_ACTOR_H
#define WATERYSQL_DROP_DATABASE_ACTOR_H

#include <string>
#include <string_view>

#include "../utility/io/printer.h"
#include "../system/system_manager.h"

namespace watery {

struct DropDatabaseActor {
    
    std::string name;
    
    explicit DropDatabaseActor(std::string_view n) noexcept
        : name{n} {}
    
    void operator()() const {
        Printer::println(std::cout, "DROP DATABASE ", name, ";");
        SystemManager::instance().drop_database(name);
    }
    
};

}

#endif  // WATERYSQL_DROP_DATABASE_ACTOR_H
