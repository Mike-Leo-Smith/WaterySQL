//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_DATA_DESCRIPTOR_H
#define WATERYSQL_DATA_DESCRIPTOR_H

#include "type_tag.h"

namespace watery {

struct DataDescriptor {
    TypeTag type;
    uint16_t length;
};

}

#endif  // WATERYSQL_DATA_DESCRIPTOR_H
