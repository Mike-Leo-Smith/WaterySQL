//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DESCRIBE_TABLE_ACTOR_H
#define WATERYSQL_DESCRIBE_TABLE_ACTOR_H

#include <string>
#include <string_view>

namespace watery {

struct DescribeTableActor {
    
    const std::string name;
    
    explicit DescribeTableActor(std::string_view name) : name{name} {}
    
    void operator()() const {
    
    }
    
};

}

#endif  // WATERYSQL_DESCRIBE_TABLE_ACTOR_H
