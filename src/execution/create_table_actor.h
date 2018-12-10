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
    
    CreateTableActor(std::string_view name, uint32_t field_count, std::array<FieldDescriptor, MAX_FIELD_COUNT> fields)
        : name{name}, descriptor{field_count, fields} {}
    
    void operator()() const {
    
    }
    
};

}

#endif  // WATERYSQL_CREATE_TABLE_ACTOR_H
