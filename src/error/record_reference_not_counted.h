//
// Created by Mike Smith on 2018-12-29.
//

#ifndef WATERYSQL_RECORD_REFERENCE_NOT_COUNTED_H
#define WATERYSQL_RECORD_REFERENCE_NOT_COUNTED_H

#include <string>
#include "error.h"

namespace watery {

struct RecordReferenceNotCounted : public Error {

    explicit RecordReferenceNotCounted(const std::string &table)
        : Error{
        "RecordReferenceNotCounted",
        std::string{"Failed to get record reference count in table \""}
            .append(table)
            .append("\" since it is not counted.")} {}

};

}

#endif  // WATERYSQL_RECORD_REFERENCE_NOT_COUNTED_H
