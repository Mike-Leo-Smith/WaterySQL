//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_CREATE_INDEX_ACTOR_H
#define WATERYSQL_CREATE_INDEX_ACTOR_H

#include <string>
#include <string_view>

namespace watery {

struct CreateIndexActor {
    
    Identifier table_name{0};
    Identifier column_name{0};
    
    CreateIndexActor(std::string_view tab, std::string_view col) noexcept {
        StringViewCopier::copy(tab, table_name);
        StringViewCopier::copy(col, column_name);
    }
    
    void operator()() const {}
    
};

}

#endif  // WATERYSQL_CREATE_INDEX_ACTOR_H
