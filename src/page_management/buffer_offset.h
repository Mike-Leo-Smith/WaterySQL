//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_BUFFER_OFFSET_H
#define WATERYSQL_BUFFER_OFFSET_H

#include "../config/config.h"

namespace watery {

struct BufferOffset {
    FileHandle file_handle;
    PageOffset page_offset;
};

}

#endif  // WATERYSQL_BUFFER_OFFSET_H
