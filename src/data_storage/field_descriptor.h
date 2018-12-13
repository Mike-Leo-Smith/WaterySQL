//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_FIELD_DESCRIPTOR_H
#define WATERYSQL_FIELD_DESCRIPTOR_H

#include <cstdint>

#include "../config/config.h"
#include "data_descriptor.h"
#include "../utility/type_constraints/non_copyable.h"
#include "field_constraint.h"
#include "../record_management/record_offset.h"

namespace watery {

struct FieldDescriptor final {
    
    char name[MAX_IDENTIFIER_LENGTH]{};
    DataDescriptor data_descriptor{};
    FieldConstraint constraints{};
    char foreign_table_name[MAX_IDENTIFIER_LENGTH]{};
    char foreign_column_name[MAX_IDENTIFIER_LENGTH]{};
    
    constexpr uint32_t record_field_length() const noexcept {
        return data_descriptor.length();
    }
    
    constexpr uint32_t index_key_length() const noexcept {
        return constraints.unique() ?
               data_descriptor.length() :
               // rid will be composed into index key to make it unique when it's actually not
               data_descriptor.length() + sizeof(RecordOffset);
    }
    
    FieldDescriptor() = default;
    
    FieldDescriptor(std::string_view n, DataDescriptor desc, FieldConstraint c)
        : data_descriptor{desc}, constraints{c} {
        n.copy(name, n.size());
    }
};

}

#endif  // WATERYSQL_FIELD_DESCRIPTOR_H

