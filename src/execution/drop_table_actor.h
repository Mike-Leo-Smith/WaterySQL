//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DROP_TABLE_ACTOR_H
#define WATERYSQL_DROP_TABLE_ACTOR_H

#include <string>
#include <string_view>

namespace watery {

struct DropTableActor {
    
    char name[MAX_IDENTIFIER_LENGTH + 1]{};
    
    explicit DropTableActor(std::string_view n) noexcept {
        n.copy(name, n.size());
    }
    
    void operator()() const {
    
    }
    
};

}

#endif  // WATERYSQL_DROP_TABLE_ACTOR_H
