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
    
    Identifier name{0};
    RecordDescriptor descriptor{};
    
    explicit CreateTableActor(std::string_view n) noexcept {
        StringViewCopier::copy(n, name);
    }
    
    void operator()() const {
        Printer::println(std::cout, "CREATE TABLE ", name.data());
        RecordDescriptorPrinter::print(std::cout, descriptor);
        SystemManager::instance().create_table(name.data(), descriptor);
    }
    
};

}

#endif  // WATERYSQL_CREATE_TABLE_ACTOR_H
