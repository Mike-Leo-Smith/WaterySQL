//
// Created by Mike Smith on 2018-12-10.
//

#ifndef WATERYSQL_CREATE_TABLE_ACTOR_H
#define WATERYSQL_CREATE_TABLE_ACTOR_H

#include "../data_storage/record_descriptor.h"

namespace watery {

struct CreateTableActor {
    
    const std::string name;
    const RecordDescriptor descriptor;
    
    CreateTableActor(std::string_view name, const std::vector<FieldDescriptor> &fields)
        : name{name}, descriptor{fields} {}
    
    void operator()() const {
        std::for_each(
            descriptor.field_descriptors.begin(),
            descriptor.field_descriptors.begin() + descriptor.field_count,
            [](FieldDescriptor fd) {
                std::cout << fd.name << ": "
                          << fd.data_descriptor.type
                          << "(" << fd.data_descriptor.size_hint << ") | "
                          << (fd.constraints.nullable() ? "NULL " : "NOT NULL ");
                if (fd.constraints.foreign()) {
                    std::cout << "| FOREIGN KEY REFERENCES "
                              << fd.foreign_table_name
                              << "(" << fd.foreign_column_name << ") ";
                }
                if (fd.constraints.primary()) {
                    std::cout << "| PRIMARY KEY ";
                }
                if (fd.constraints.unique()) {
                    std::cout << "| UNIQUE ";
                }
                std::cout << std::endl;
            });
    }
    
};

}

#endif  // WATERYSQL_CREATE_TABLE_ACTOR_H
