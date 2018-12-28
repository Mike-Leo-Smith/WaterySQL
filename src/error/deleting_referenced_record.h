//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_DELETING_REFERENCED_RECORD_H
#define WATERYSQL_DELETING_REFERENCED_RECORD_H

#include "error.h"

namespace watery {

struct DeletingReferencedRecord : public Error {
    
    explicit DeletingReferencedRecord(const std::string &table)
        : Error{
        "DeletingReferencedRecord",
        std::string{"Failed to delete record in table \""}
            .append(table).append("\" which is referenced by other records.")} {}
    
};

}

#endif  // WATERYSQL_DELETING_REFERENCED_RECORD_H
