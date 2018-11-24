//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_FIELD_DESCRIPTOR_H
#define WATERYSQL_FIELD_DESCRIPTOR_H

#include <cstdint>

#include "../config/config.h"
#include "../data_storage/data_descriptor.h"
#include "../utility/type_constraints/noncopyable.h"

namespace watery {

struct FieldDescriptor final {
    uint8_t name[MAX_FIELD_NAME_LENGTH];
    DataDescriptor data_descriptor;
};

}

#endif  // WATERYSQL_FIELD_DESCRIPTOR_H
