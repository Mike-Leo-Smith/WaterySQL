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
    
    Identifier name{0};
    
    explicit DropDatabaseActor(std::string_view n) noexcept {
        StringViewCopier::copy(n, name);
    }
    
    void operator()() const {
        Printer::println(std::cout, "DROP DATABASE ", name.data(), ";");
        SystemManager::instance().drop_database(name.data());
    }
    
};

}

#endif  // WATERYSQL_DROP_DATABASE_ACTOR_H
