//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_NEGATIVE_FOREIGN_KEY_REFERENCE_COUNT_H
#define WATERYSQL_NEGATIVE_FOREIGN_KEY_REFERENCE_COUNT_H

#include "error.h"

namespace watery {

struct NegativeForeignKeyReferenceCount : public Error {
    
    NegativeForeignKeyReferenceCount(const std::string &table_name, const std::string &removed_referrer)
        : Error{
        "NegativeForeignKeyReferenceCount",
        std::string{"Foreign key reference count of table \""}
            .append(table_name)
            .append("\" drops to negative when removing reference from table \"")
            .append(removed_referrer).append("\".")} {}
    
};

}

#endif  // WATERYSQL_NEGATIVE_FOREIGN_KEY_REFERENCE_COUNT_H
