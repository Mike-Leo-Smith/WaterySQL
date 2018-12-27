//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_INDEX_ENTRY_OFFSET_OUT_OF_RANGE_H
#define WATERYSQL_INDEX_ENTRY_OFFSET_OUT_OF_RANGE_H

#include "error.h"
#include "../index/index_entry_offset.h"

namespace watery {

struct IndexEntryOffsetOutOfRange : public Error {
    
    IndexEntryOffsetOutOfRange(const std::string &index_name, IndexEntryOffset offset)
        : Error{
        "IndexEntryOffsetOutOfRange",
        std::string{"Given index entry offset "}
            .append(offset.to_string()).append(" is invalid in index \"")
            .append(index_name).append("\".")} {}
    
};

}

#endif  // WATERYSQL_INDEX_ENTRY_OFFSET_OUT_OF_RANGE_H
