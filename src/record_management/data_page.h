//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_DATA_PAGE_H
#define WATERYSQL_DATA_PAGE_H

#include "data_page_header.h"

namespace watery {

struct DataPage {
    DataPageHeader header;
    Byte data[];
};

}

#endif  // WATERYSQL_DATA_PAGE_H
