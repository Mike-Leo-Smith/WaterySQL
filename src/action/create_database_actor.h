//
// Created by Mike Smith on 2018-12-07.
//

#ifndef WATERYSQL_CREATE_DATABASE_ACTOR_H
#define WATERYSQL_CREATE_DATABASE_ACTOR_H

#include <string>
#include <string_view>
#include "../system/system_manager.h"
#include "../utility/io/printer.h"

namespace watery {

struct CreateDatabaseActor {
    
    std::string name;
    
    explicit CreateDatabaseActor(std::string_view n) noexcept
        : name{n} {}
    
    void operator()() const {
        Printer::println(std::cout, "CREATE DATABASE ", name);
        SystemManager::instance().create_database(name);
        Printer::println(std::cout, "Done.");
    }
};

}

#endif  // WATERYSQL_CREATE_DATABASE_ACTOR_H
