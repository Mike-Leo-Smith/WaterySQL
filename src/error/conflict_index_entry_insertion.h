//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_CONFLICT_INDEX_ENTRY_INSERTION_H
#define WATERYSQL_CONFLICT_INDEX_ENTRY_INSERTION_H

#include "error.h"

namespace watery {

struct ConflictIndexEntryInsertion : public Error {
    
    explicit ConflictIndexEntryInsertion(const std::string &name)
        : Error {
        "ConflictIndexEntryInsertion",
        std::string{"Insertion into index \""}.append(name).append("\" caused conflicts.")} {}
    
};

}

#endif  // WATERYSQL_CONFLICT_INDEX_ENTRY_INSERTION_H
