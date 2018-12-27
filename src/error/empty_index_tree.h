//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_EMPTY_INDEX_TREE_H
#define WATERYSQL_EMPTY_INDEX_TREE_H

#include "error.h"

namespace watery {

struct EmptyIndexTree : public Error {
    
    explicit EmptyIndexTree(const std::string &index_name)
        : Error{
        "EmptyIndexTree",
        std::string{"Operations cannot be done in index \""}
            .append(index_name).append("\" which is empty.")} {}
};

}

#endif  // WATERYSQL_EMPTY_INDEX_TREE_H
