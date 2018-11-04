//
// Created by Mike Smith on 2018/11/4.
//

#include "record.h"

namespace watery {

Record::Record(uint32_t id, uint32_t field_count)
    : _id{id} {
    _fields.resize(field_count);
}

void Record::set_field(int index, std::unique_ptr<Data> &&field) {
    _fields[index] = std::move(field);
}

const std::unique_ptr<Data> &Record::get_field(int index) {
    return _fields[index];
}

Record Record::decode(const RecordDescriptor &descriptor, const uint8_t *raw) {
    auto &&record = Record{*reinterpret_cast<const uint32_t *>(raw), descriptor.field_count};
    auto ptr = raw + sizeof(uint32_t);
    for (auto i = 0; i < descriptor.field_count; i++) {
        auto &&data_descriptor = descriptor.field_descriptors[i].data_descriptor;
        record.set_field(i, Data::decode(data_descriptor, ptr));
        ptr += data_descriptor.size;
    }
    return std::move(record);
}

void Record::encode(uint8_t *buffer) const {
    *reinterpret_cast<uint32_t *>(buffer) = _id;
    buffer += sizeof(uint32_t);
    for (auto &&data: _fields) {
        data->encode(buffer);
        buffer += data->descriptor().size;
    }
}

uint32_t Record::id() const {
    return _id;
}

uint32_t Record::slot() const {
    return _slot;
}

}
