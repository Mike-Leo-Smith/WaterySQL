//
// Created by Mike Smith on 2018-12-28.
//

#ifndef WATERYSQL_NEGATIVE_RECORD_REFERENCE_COUNT_H
#define WATERYSQL_NEGATIVE_RECORD_REFERENCE_COUNT_H

#include "error.h"

namespace watery {

struct NegativeRecordReferenceCount : public Error {
    explicit NegativeRecordReferenceCount(const std::string &table)
        : Error{
        "NegativeRecordReferenceCount",
        std::string{"Reference count of primary key record in table \""}
            .append(table).append("\" drops negative.")} {}
};

}

#endif  // WATERYSQL_NEGATIVE_RECORD_REFERENCE_COUNT_H
