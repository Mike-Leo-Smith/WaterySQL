//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_INDEX_ENTRY_OVERSIZED_H
#define WATERYSQL_INDEX_ENTRY_OVERSIZED_H

#include "error.h"

namespace watery {

struct IndexEntryOversized : public Error {
    
    IndexEntryOversized(const std::string &index, uint32_t len)
        : Error{
        "IndexEntryOversized",
        std::string{"Entries of index \""}
            .append(index).append("\" with size ")
            .append(std::to_string(len)).append(" bytes are too long to fit into a page.")} {}
    
};

}

#endif  // WATERYSQL_INDEX_ENTRY_OVERSIZED_H
