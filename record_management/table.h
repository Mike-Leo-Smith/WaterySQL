#include <utility>

//
// Created by Mike Smith on 2018/11/5.
//

#ifndef WATERYSQL_TABLE_H
#define WATERYSQL_TABLE_H

#include <string>
#include <unordered_set>
#include "../utility/noncopyable.h"
#include "record_descriptor.h"
#include "../filesystem_demo/utils/pagedef.h"

namespace watery {

class Table : Noncopyable {
public:
    std::string name;
    RecordDescriptor record_descriptor;
    int32_t file_id;
    uint32_t page_count;
    uint32_t record_count;
    uint32_t record_length;
    uint32_t slot_count_per_page;
    int32_t current_rid;
    std::unordered_set<int32_t> buffer_ids;
    
    Table(std::string name, const RecordDescriptor &rd, int32_t fid = -1,
          int32_t curr_rid = 0, uint32_t pc = 0, uint32_t rc = 0)
        : name{std::move(name)}, record_descriptor{rd}, file_id{fid}, current_rid{curr_rid},
          page_count{pc}, record_count{rc}, record_length{rd.length()}, slot_count_per_page{
            (PAGE_SIZE - SLOT_BITSET_SIZE - static_cast<uint32_t>(sizeof(uint32_t))) / record_length} {}
};

}

#endif  // WATERYSQL_TABLE_H
