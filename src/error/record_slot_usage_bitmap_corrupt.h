//
// Created by Mike Smith on 2018-12-27.
//

#ifndef WATERYSQL_RECORD_SLOT_USAGE_BITMAP_CORRUPT_H
#define WATERYSQL_RECORD_SLOT_USAGE_BITMAP_CORRUPT_H

#include "error.h"
#include "../config/config.h"

namespace watery {

struct RecordSlotUsageBitmapCorrupt : public Error {
    
    RecordSlotUsageBitmapCorrupt(const std::string &table_name, PageOffset page)
        : Error{
        "RecordSlotUsageBitmapCorrupt",
        std::string{"Record slot usage bitmap of page #"}
            .append(std::to_string(page)).append(" in table \"")
            .append(table_name).append("\" might be corrupt.")} {}
    
};

}

#endif  // WATERYSQL_RECORD_SLOT_USAGE_BITMAP_CORRUPT_H
