//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_RECORD_NOT_FOUND_H
#define WATERYSQL_RECORD_NOT_FOUND_H

#include "error.h"
#include "../record/record_offset.h"

namespace watery {

struct RecordNotFound : public Error {
    RecordNotFound(const std::string &table_name, RecordOffset rid)
        : Error{
        "RecordNotFound",
        std::string{"Given record offset "}
            .append(rid.to_string())
            .append(" does not refer to a existent record in table \"")
            .append(table_name).append("\".")} {}
};

}

#endif  // WATERYSQL_RECORD_NOT_FOUND_H
