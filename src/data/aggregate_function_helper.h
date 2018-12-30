//
// Created by Mike Smith on 2018-12-30.
//

#ifndef WATERYSQL_AGGREGATE_FUNCTION_HELPER_H
#define WATERYSQL_AGGREGATE_FUNCTION_HELPER_H

#include <string_view>

#include "aggregate_function.h"
#include "../utility/type/non_constructible.h"

namespace watery {

struct AggregateFunctionHelper : NonConstructible {
    
    static constexpr std::string_view name(AggregateFunction f) noexcept {
        switch (f) {
            case AggregateFunction::SUM:
                return "SUM";
            case AggregateFunction::AVERAGE:
                return "AVG";
            case AggregateFunction::MIN:
                return "MIN";
            case AggregateFunction::MAX:
                return "MAX";
            case AggregateFunction::COUNT:
                return "COUNT";
            default:
                return "NONE";
        }
    }
    
};

}

#endif  // WATERYSQL_AGGREGATE_FUNCTION_HELPER_H
