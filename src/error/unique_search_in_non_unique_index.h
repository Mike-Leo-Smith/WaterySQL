//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_UNIQUE_SEARCH_IN_NON_UNIQUE_INDEX_H
#define WATERYSQL_UNIQUE_SEARCH_IN_NON_UNIQUE_INDEX_H

#include "error.h"

namespace watery {

struct UniqueSearchInNonUniqueIndex : public Error {
    
    explicit UniqueSearchInNonUniqueIndex(const std::string &name)
        : Error{
        "UniqueSearchInNonUniqueIndex",
        std::string{"Cannot do unique search in non-unique index \""}.append(name).append("\".")} {}
    
};

}

#endif  // WATERYSQL_UNIQUE_SEARCH_IN_NON_UNIQUE_INDEX_H
