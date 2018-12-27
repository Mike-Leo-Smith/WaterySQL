//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_DESCRIBE_TABLE_ACTOR_H
#define WATERYSQL_DESCRIBE_TABLE_ACTOR_H

#include <string>
#include <string_view>

#include "../utility/io/printer.h"
#include "../utility/io/record_descriptor_printer.h"

namespace watery {

struct DescribeTableActor {
    
    std::string name;
    
    explicit DescribeTableActor(std::string_view n) noexcept
        : name{n} {}
    
    void operator()() const {
        Printer::println(std::cout, "DESCRIBE TABLE ", name, ";");
        RecordDescriptorPrinter::print(std::cout, SystemManager::instance().describe_table(name));
    }
    
};

}

#endif  // WATERYSQL_DESCRIBE_TABLE_ACTOR_H
