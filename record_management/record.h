//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_RECORD_H
#define WATERYSQL_RECORD_H

#include <cstdint>
#include <vector>

#include "../data_storage/data.h"
#include "../config/config.h"
#include "record_descriptor.h"

namespace watery {

class Record : Noncopyable {
private:
    int32_t _id;
    int32_t _slot;
public:
    void set_slot(int32_t slot);
private:
    std::array<std::unique_ptr<Data>, MAX_FIELD_COUNT> _fields;
    
public:
    Record(uint32_t field_count, int32_t id, int32_t slot = -1);
    void set_field(int index, std::unique_ptr<Data> field);
    const std::unique_ptr<Data> &get_field(int index);
    
    int32_t id() const;
    int32_t slot() const;
    const std::array<std::unique_ptr<Data>, MAX_FIELD_COUNT> &fields() const;
};

}

#endif  // WATERYSQL_RECORD_H
