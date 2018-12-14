//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DROP_INDEX_ACTOR_H
#define WATERYSQL_DROP_INDEX_ACTOR_H

#include <string>
#include <string_view>

namespace watery {

struct DropIndexActor {
    
    char table_name [MAX_IDENTIFIER_LENGTH + 1]{};
    char column_name[MAX_IDENTIFIER_LENGTH + 1]{};
    
    DropIndexActor(std::string_view tab, std::string_view col) noexcept {
        tab.copy(table_name, tab.size());
        col.copy(column_name, col.size());
    }
    
    void operator()() const {
    
    }
    
};

}

#endif  // WATERYSQL_DROP_INDEX_ACTOR_H
