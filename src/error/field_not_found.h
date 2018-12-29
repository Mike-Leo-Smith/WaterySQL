//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_FIELD_NOT_FOUND_H
#define WATERYSQL_FIELD_NOT_FOUND_H

#include "error.h"

namespace watery {

struct FieldNotFound : public Error {
    FieldNotFound(const std::string &table, const std::string &field)
        : Error{
        "FieldNotFound",
        std::string{"Cannot find field \""}
            .append(field).append("\" in table \"").append(table).append("\".")} {}
};

}

#endif  // WATERYSQL_FIELD_NOT_FOUND_H
