//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_INDEX_ENTRY_NOT_FOUND_H
#define WATERYSQL_INDEX_ENTRY_NOT_FOUND_H

#include "error.h"

namespace watery {

struct IndexEntryNotFound : public Error {
    
    explicit IndexEntryNotFound(const std::string &name)
        : Error{
        "IndexEntryNotFound",
        std::string{"Index entry not found in index \""}.append(name).append("\".")} {}
    
};

}

#endif  // WATERYSQL_INDEX_ENTRY_NOT_FOUND_H
