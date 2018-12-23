//
// Created by Mike Smith on 2018-12-23.
//

#ifndef WATERYSQL_EXIT_ACTOR_H
#define WATERYSQL_EXIT_ACTOR_H

#include "../system/system_manager.h"
#include "../utility/io/printer.h"

namespace watery {

struct ExitActor {
    void operator()() const noexcept {
        Printer::println(std::cout, "Bye.");
        SystemManager::instance().quit();
    }
};

}

#endif  // WATERYSQL_EXIT_ACTOR_H
