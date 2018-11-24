//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_PAGE_H
#define WATERYSQL_PAGE_H

#include <cstdint>
#include "../config/config.h"

namespace watery {

struct Page {
    FileHandle file_handle;
    PageOffset page_offset;
    PageHandle page_handle;
    Byte *buffer;
};

}

#endif  // WATERYSQL_PAGE_H
