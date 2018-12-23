//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_FIELD_DESCRIPTOR_H
#define WATERYSQL_FIELD_DESCRIPTOR_H

#include <cstdint>

#include "../config/config.h"
#include "data_descriptor.h"
#include "../utility/type/non_copyable.h"
#include "field_constraint.h"
#include "../record/record_offset.h"
#include "../utility/memory/string_view_copier.h"

namespace watery {

struct FieldDescriptor final {
    
    Identifier name{};
    DataDescriptor data_descriptor{};
    FieldConstraint constraints{};
    Identifier foreign_table_name{};
    Identifier foreign_column_name{};
    bool indexed{false};
    
    FieldDescriptor() = default;
    
    FieldDescriptor(std::string_view n, DataDescriptor desc, FieldConstraint c)
        : data_descriptor{desc}, constraints{c} {
        StringViewCopier::copy(n, name);
    }
};

}

#endif  // WATERYSQL_FIELD_DESCRIPTOR_H

