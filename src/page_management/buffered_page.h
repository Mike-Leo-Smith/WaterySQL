//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_PAGE_HANDLE_H
#define WATERYSQL_PAGE_HANDLE_H

#include <cstdint>
#include "../config/config.h"
#include "buffer_offset.h"

namespace watery {

struct PageHandle {
    BufferOffset buffer_offset;
    BufferHandle buffer_handle;
    Byte *data;
};

}

#endif  // WATERYSQL_PAGE_HANDLE_H
