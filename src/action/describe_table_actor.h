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
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::println(std::cout, "DESCRIBE TABLE ", name);
        }
        auto[ms, desc] = timed_run([name = name] {
            return SystemManager::instance().describe_table(name);
        });
        
        std::ofstream f{RESULT_FILE_NAME, std::ios::app};
        RecordDescriptorPrinter::print(f, desc);
        Printer::println(f, "Done in ", ms, "ms.<br/>");
    }
    
};

}

#endif  // WATERYSQL_DESCRIBE_TABLE_ACTOR_H
