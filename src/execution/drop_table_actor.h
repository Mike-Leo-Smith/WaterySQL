//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DROP_TABLE_ACTOR_H
#define WATERYSQL_DROP_TABLE_ACTOR_H

#include <string>
#include <string_view>

namespace watery {

struct DropTableActor {
    
    const std::string name;
    
    explicit DropTableActor(std::string_view name) : name{name} {}
    
    void operator()() const {
    
    }
    
};

}

#endif  // WATERYSQL_DROP_TABLE_ACTOR_H
