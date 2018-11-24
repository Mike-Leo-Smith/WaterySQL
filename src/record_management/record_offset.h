//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_RECORD_OFFSET_H
#define WATERYSQL_RECORD_OFFSET_H

#include "../config/config.h"

namespace watery {

struct RecordOffset {
    PageOffset page_offset;
    SlotOffset slot_offset;
};

}

#endif  // WATERYSQL_RECORD_OFFSET_H
