//
// Created by Mike Smith on 2018-12-29.
//

#ifndef WATERYSQL_INVALID_PRIMARY_KEY_UPDATE_H
#define WATERYSQL_INVALID_PRIMARY_KEY_UPDATE_H

#include <string>
#include "error.h"

namespace watery {

struct InvalidPrimaryKeyUpdate : public Error {

    InvalidPrimaryKeyUpdate(const std::string &table, const std::string &col)
        : Error{
        "InvalidPrimaryKeyUpdate",
        std::string{"Updating primary key column \""}
            .append(col)
            .append("\" in table \"")
            .append(table)
            .append("\" is not allowed.")} {}

};

}

#endif  // WATERYSQL_INVALID_PRIMARY_KEY_UPDATE_H
