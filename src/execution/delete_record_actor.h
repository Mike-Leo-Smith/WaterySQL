#include <utility>

//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_DELETE_RECORD_ACTOR_H
#define WATERYSQL_DELETE_RECORD_ACTOR_H

#include <vector>

#include "../config/config.h"
#include "column_predicate.h"
#include "../utility/memory/string_view_copier.h"

namespace watery {

struct DeleteRecordActor {
    
    char table_name[MAX_FIELD_COUNT + 1]{0};
    std::vector<ColumnPredicate> predicates;
    
    explicit DeleteRecordActor(std::string_view tab, std::vector<ColumnPredicate> pred) noexcept
        : predicates{std::move(pred)} {
        StringViewCopier::copy(tab, table_name);
    }
    
    void operator()() const {
    
    }
    
};

}

#endif  // WATERYSQL_DELETE_RECORD_ACTOR_H
