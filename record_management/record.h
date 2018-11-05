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

class Record {
private:
    int32_t _id;
    
private:
    std::vector<std::unique_ptr<Data>> _fields;

public:
    Record(int32_t id, uint32_t field_count);
    void set_field(int index, std::unique_ptr<Data> &&field);
    const std::unique_ptr<Data> &get_field(int index);
    
    int32_t id() const;
    
    static Record decode(const RecordDescriptor &descriptor, const uint8_t *raw);
    void encode(uint8_t *buffer) const;
    
};

}

#endif  // WATERYSQL_RECORD_H
