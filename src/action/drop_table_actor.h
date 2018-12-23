//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DROP_TABLE_ACTOR_H
#define WATERYSQL_DROP_TABLE_ACTOR_H

#include <string>
#include <string_view>

#include "../utility/io/printer.h"
#include "../system/system_manager.h"

namespace watery {

struct DropTableActor {
    
    Identifier name{0};
    
    explicit DropTableActor(std::string_view n) noexcept {
        StringViewCopier::copy(n, name);
    }
    
    void operator()() const {
        Printer::println(std::cout, "DROP TABLE ", name.data(), ";");
        SystemManager::instance().drop_table(name.data());
    }
    
};

}

#endif  // WATERYSQL_DROP_TABLE_ACTOR_H
