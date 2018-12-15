//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DROP_INDEX_ACTOR_H
#define WATERYSQL_DROP_INDEX_ACTOR_H

#include <string>
#include <string_view>

namespace watery {

struct DropIndexActor {
    
    char table_name [MAX_IDENTIFIER_LENGTH + 1]{0};
    char column_name[MAX_IDENTIFIER_LENGTH + 1]{0};
    
    DropIndexActor(std::string_view tab, std::string_view col) noexcept {
        StringViewCopier::copy(tab, table_name);
        StringViewCopier::copy(col, column_name);
    }
    
    void operator()() const {
    
    }
    
};

}

#endif  // WATERYSQL_DROP_INDEX_ACTOR_H
