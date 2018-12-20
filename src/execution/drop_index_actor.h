//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DROP_INDEX_ACTOR_H
#define WATERYSQL_DROP_INDEX_ACTOR_H

#include <string>
#include <string_view>

#include "../utility/io/printer.h"

namespace watery {

struct DropIndexActor {
    
    Identifier table_name{0};
    Identifier column_name{0};
    
    DropIndexActor(std::string_view tab, std::string_view col) noexcept {
        StringViewCopier::copy(tab, table_name);
        StringViewCopier::copy(col, column_name);
    }
    
    void operator()() const {
        Printer::println(std::cout, "DROP INDEX ", table_name.data(), "(", column_name.data(), ");");
    }
    
};

}

#endif  // WATERYSQL_DROP_INDEX_ACTOR_H
