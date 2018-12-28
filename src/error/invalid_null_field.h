//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_INVALID_NULL_FIELD_H
#define WATERYSQL_INVALID_NULL_FIELD_H

#include "error.h"

namespace watery {

struct InvalidNullField : public Error {
    
    InvalidNullField(const std::string &table, const std::string &field)
        : Error{
        "InvalidNullField",
        std::string{"Cannot assign NULL to field \""}
        .append(field).append("\" in table \"")
        .append(table).append("\" which is under NOT NULL constraint.")} {}
    
};

}

#endif  // WATERYSQL_INVALID_NULL_FIELD_H
