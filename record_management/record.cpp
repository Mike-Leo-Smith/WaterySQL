//
// Created by Mike Smith on 2018/11/4.
//

#include "record.h"

namespace watery {

Record::Record(uint32_t field_count, int32_t id, int32_t slot)
    : _id{id}, _slot{slot} {}

void Record::set_field(int index, std::unique_ptr<Data> field) {
    _fields[index] = std::move(field);
}

const std::unique_ptr<Data> &Record::get_field(int index) {
    return _fields[index];
}

int32_t Record::id() const {
    return _id;
}

int32_t Record::slot() const {
    return _slot;
}

const std::array<std::unique_ptr<Data>, 32> &Record::fields() const {
    return _fields;
}

void Record::set_slot(int32_t slot) {
    _slot = slot;
}

}
