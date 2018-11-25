//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_DATA_PAGE_H
#define WATERYSQL_DATA_PAGE_H

#include <array>

#include "data_page_header.h"
#include "../utility/type_constraints/non_copyable.h"
#include "../utility/type_constraints/non_movable.h"
#include "../utility/type_constraints/non_trivial_constructible.h"

namespace watery {

struct DataPage : NonCopyable, NonTrivialConstructible, NonMovable {
    DataPageHeader header;
    Byte data[];  // serves as a position indicator.
};

}

#endif  // WATERYSQL_DATA_PAGE_H
