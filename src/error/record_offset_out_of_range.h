//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_RECORD_OFFSET_OUT_OF_RANGE_H
#define WATERYSQL_RECORD_OFFSET_OUT_OF_RANGE_H

#include "error.h"
#include "../record/record_offset.h"

namespace watery {

struct RecordOffsetOutOfRange : public Error {
    
    explicit RecordOffsetOutOfRange(const std::string &table_name, RecordOffset rid)
        : Error{
        "RecordOffsetOutOfRange",
        std::string{"Given record offset "}
            .append(rid.to_string()).append(" is invalid in table \"").append(table_name).append("\".")} {}
};

}

#endif  // WATERYSQL_RECORD_OFFSET_OUT_OF_RANGE_H
