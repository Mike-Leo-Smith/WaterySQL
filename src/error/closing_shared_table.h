//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_CLOSING_SHARED_TABLE_H
#define WATERYSQL_CLOSING_SHARED_TABLE_H

#include "error.h"

namespace watery {

struct ClosingSharedTable : public Error {
    
    explicit ClosingSharedTable(const std::string &name)
        : Error{
        "ClosingSharedTable",
        std::string{"Attempt to close table \""}
            .append(name).append("\" is not allowed when it is still shared.")} {}
    
};

}

#endif  // WATERYSQL_CLOSING_SHARED_TABLE_H
