//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_CREATE_TABLE_ACTOR_H
#define WATERYSQL_CREATE_TABLE_ACTOR_H

#include <iostream>

#include "../data_storage/record_descriptor.h"
#include "../utility/io/identifier_printing.h"
#include "../system_management/system_manager.h"

namespace watery {

struct CreateTableActor {
    
    Identifier name{0};
    RecordDescriptor descriptor;
    
    explicit CreateTableActor(std::string_view n) noexcept {
        StringViewCopier::copy(n, name);
    }
    
    void operator()() const {
        print_info();
        SystemManager::instance().create_table(name.data(), descriptor);
    }
    
    void print_info() const {
        Printer::println(std::cout, "CREATE TABLE ", name);
        std::for_each(
            descriptor.field_descriptors.begin(),
            descriptor.field_descriptors.begin() + descriptor.field_count,
            [](FieldDescriptor fd) {
                Printer::print(
                    std::cout, "  ", fd.name, ": ", fd.data_descriptor.type,
                    "(", fd.data_descriptor.size_hint, ") | ",
                    fd.constraints.nullable() ? "NULL " : "NOT NULL ");
                if (fd.constraints.foreign()) {
                    Printer::print(
                        std::cout, "| FOREIGN KEY REFERENCES ", fd.foreign_table_name,
                        "(", fd.foreign_column_name, ") ");
                }
                if (fd.constraints.primary()) {
                    Printer::print(std::cout, "| PRIMARY KEY ");
                }
                if (fd.constraints.unique()) {
                    Printer::print(std::cout, "| UNIQUE ");
                }
                Printer::print(std::cout, "\n");
            });
        Printer::println(std::cout);
    }
    
};

}

#endif  // WATERYSQL_CREATE_TABLE_ACTOR_H
