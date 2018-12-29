//
// Created by Mike Smith on 2018-12-29.
//

#ifndef WATERYSQL_CONFLICT_RECORD_FIELD_UPDATE_H
#define WATERYSQL_CONFLICT_RECORD_FIELD_UPDATE_H

#include <string>
#include "error.h"

namespace watery {

struct ConflictRecordFieldUpdate : public Error {

    ConflictRecordFieldUpdate(const std::string &table, const std::string &column)
        : Error{
        "ConflictRecordFieldUpdate",
        std::string{"Cannot assign multiple values to column \""}
            .append(column)
            .append("\" when updating table \"")
            .append(table)
            .append("\".")} {}

};

}

#endif  // WATERYSQL_CONFLICT_RECORD_FIELD_UPDATE_H
