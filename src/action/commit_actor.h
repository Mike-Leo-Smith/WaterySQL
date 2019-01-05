//
// Created by Mike Smith on 2019-01-05.
//

#ifndef WATERYSQL_COMMIT_ACTOR_H
#define WATERYSQL_COMMIT_ACTOR_H

#include "../system/system_manager.h"
#include "../utility/time/elapsed_time.h"

namespace watery {

struct CommitActor {
    
    void operator()() const noexcept {
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::println(f, "COMMIT");
        }
        auto ms = timed_run([] { SystemManager::instance().commit(); }).first;
        std::ofstream f{RESULT_FILE_NAME, std::ios::app};
        Printer::println(f, "Done in ", ms, "ms.");
    }
    
};

}

#endif  // WATERYSQL_COMMIT_ACTOR_H
