//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_PREDICATE_EFFICIENCY_ESTIMATOR_H
#define WATERYSQL_PREDICATE_EFFICIENCY_ESTIMATOR_H

#include "../record/record_manager.h"
#include "../action/column_predicate.h"

namespace watery {

class PredicateEfficiencyEstimator {

private:
    RecordManager &_record_manager{RecordManager::instance()};

public:
    int32_t score(const ColumnPredicate &predicate);
};

}

#endif  // WATERYSQL_PREDICATE_EFFICIENCY_ESTIMATOR_H
