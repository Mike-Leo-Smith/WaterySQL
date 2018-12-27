//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_CLOSING_SHARED_INDEX_H
#define WATERYSQL_CLOSING_SHARED_INDEX_H

#include "error.h"

namespace watery {

struct ClosingSharedIndex : public Error {
    
    explicit ClosingSharedIndex(const std::string &name)
        : Error{
        "ClosingSharedIndex",
        std::string{"Attempt to close index \""}
            .append(name).append("\" is not allowed when it is still shared.")} {}
    
};

}

#endif  // WATERYSQL_CLOSING_SHARED_INDEX_H
