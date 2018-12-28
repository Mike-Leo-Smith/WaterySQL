//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_TOO_MANY_COLUMNS_TO_INSERT_H
#define WATERYSQL_TOO_MANY_COLUMNS_TO_INSERT_H

#include "error.h"

namespace watery {

struct TooManyColumnsToInsert : public Error {
    
    TooManyColumnsToInsert(const std::string &table, uint32_t count)
        : Error{
        "TooManyColumnsToInsert",
        std::string{"Record insertion into table \""}
            .append(table).append("\" contains too many (")
            .append(std::to_string(count)).append(") columns.")} {}
    
};

}

#endif  // WATERYSQL_TOO_MANY_COLUMNS_TO_INSERT_H
