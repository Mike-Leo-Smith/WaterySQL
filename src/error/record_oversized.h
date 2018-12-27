//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_RECORD_OVERSIZED_H
#define WATERYSQL_RECORD_OVERSIZED_H

#include "error.h"

namespace watery {

struct RecordOversized : public Error {
    
    RecordOversized(const std::string &table_name, uint32_t len)
        : Error{
        "RecordOversized",
        std::string{"Records of table \""}
            .append(table_name).append("\" with size ")
            .append(std::to_string(len)).append(" are too long to fit into a data page.")} {}
    
};

}

#endif  // WATERYSQL_RECORD_OVERSIZED_H
