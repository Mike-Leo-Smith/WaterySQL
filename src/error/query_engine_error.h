//
// Created by Mike Smith on 2018-12-23.
//

#ifndef WATERYSQL_QUERY_ENGINE_ERROR_H
#define WATERYSQL_QUERY_ENGINE_ERROR_H

#include "error.h"

namespace watery {

struct QueryEngineError : public Error {
    explicit QueryEngineError(std::string_view reason)
        : Error("QueryEngineError", reason) {}
};

}

#endif  // WATERYSQL_QUERY_ENGINE_ERROR_H
