//
// Created by Mike Smith on 2018-12-09.
//

#ifndef WATERYSQL_FIELD_VIEW_H
#define WATERYSQL_FIELD_VIEW_H

#include "field_descriptor.h"

namespace watery {

struct FieldView {
    
    const FieldDescriptor &descriptor;
    const Byte *raw;
    
};

}

#endif  // WATERYSQL_FIELD_VIEW_H
