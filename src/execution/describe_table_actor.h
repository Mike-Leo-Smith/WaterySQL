//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DESCRIBE_TABLE_ACTOR_H
#define WATERYSQL_DESCRIBE_TABLE_ACTOR_H

#include <string>
#include <string_view>

#include "../utility/io/printer.h"

namespace watery {

struct DescribeTableActor {
    
    Identifier name{0};
    
    explicit DescribeTableActor(std::string_view n) noexcept {
        StringViewCopier::copy(n, name);
    }
    
    void operator()() const {
        Printer::println(std::cout, "DESCRIBE TABLE ", name.data(), ";");
    }
    
};

}

#endif  // WATERYSQL_DESCRIBE_TABLE_ACTOR_H
