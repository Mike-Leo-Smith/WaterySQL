//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_DATA_TYPE_H
#define WATERYSQL_DATA_TYPE_H

#include <cstdint>
#include <memory>
#include "type_tag.h"
#include "data_descriptor.h"
#include "../utility/noncopyable.h"

namespace watery {

class Data : Noncopyable {
private:
    DataDescriptor _descriptor;

protected:
    Data(DataDescriptor descriptor);

public:
    virtual ~Data() = default;
    const DataDescriptor &descriptor() const;
    static std::unique_ptr<Data> decode(DataDescriptor descriptor, const uint8_t *raw);
    virtual void encode(uint8_t *buffer) const = 0;
};

}

#endif  // WATERYSQL_DATA_TYPE_H
