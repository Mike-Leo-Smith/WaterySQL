//
// Created by Mike Smith on 2018-12-30.
//

#ifndef WATERYSQL_SELECTION_CONTEXT_H
#define WATERYSQL_SELECTION_CONTEXT_H

#include <memory>
#include <vector>

#include "../record/table.h"

namespace watery {

struct SelectionContext {
    
    std::vector<std::shared_ptr<Table>> tables;
    std::vector<std::vector<Byte>> records;
    
};

}

#endif  // WATERYSQL_SELECTION_CONTEXT_H
