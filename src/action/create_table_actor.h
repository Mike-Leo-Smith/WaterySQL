//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_CREATE_TABLE_ACTOR_H
#define WATERYSQL_CREATE_TABLE_ACTOR_H

#include <iostream>

#include "../data/record_descriptor.h"
#include "../system/system_manager.h"
#include "../utility/io/record_descriptor_printer.h"

namespace watery {

struct CreateTableActor {
    
    std::string name;
    RecordDescriptor descriptor{};
    
    explicit CreateTableActor(std::string_view n) noexcept
        : name{n} {}
    
    void operator()() const {
        Printer::println(std::cout, "CREATE TABLE ", name);
        RecordDescriptorPrinter::print(std::cout, descriptor);
        SystemManager::instance().create_table(name, descriptor);
        Printer::println(std::cout, "Done.\n");
    }
    
};

}

#endif  // WATERYSQL_CREATE_TABLE_ACTOR_H
