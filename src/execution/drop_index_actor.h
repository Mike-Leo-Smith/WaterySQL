//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DROP_INDEX_ACTOR_H
#define WATERYSQL_DROP_INDEX_ACTOR_H

#include <string>
#include <string_view>

namespace watery {

struct DropIndexActor {
    
    const std::string table_name;
    const std::string column_name;
    
    DropIndexActor(std::string_view tab, std::string_view col)
        : table_name{tab}, column_name{col} {}
    
    void operator()() const {
    
    }
    
};

}

#endif  // WATERYSQL_DROP_INDEX_ACTOR_H
