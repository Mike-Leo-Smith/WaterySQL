//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_DATA_PAGE_HEADER_H
#define WATERYSQL_DATA_PAGE_HEADER_H

#include <bitset>
#include "../config/config.h"

namespace watery {

struct DataPageHeader {
    uint32_t record_count{0};
    PageOffset next_page{0};
    std::bitset<MAX_SLOT_COUNT_PER_PAGE> slot_usage_bitmap;
};

}

#endif  // WATERYSQL_DATA_PAGE_HEADER_H
