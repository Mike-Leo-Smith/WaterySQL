//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_DATA_PAGE_H
#define WATERYSQL_DATA_PAGE_H

#include <array>

#include "data_page_header.h"
#include "../utility/type/non_copyable.h"
#include "../utility/type/non_movable.h"
#include "../utility/type/non_constructible.h"

namespace watery {

struct DataPage : NonCopyable, NonConstructible, NonMovable {
    DataPageHeader header;
    alignas(8) Byte data[1];  // serves as a position indicator.
};

}

#endif  // WATERYSQL_DATA_PAGE_H
