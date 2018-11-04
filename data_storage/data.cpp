//
// Created by Mike Smith on 2018/11/4.
//

#include "data.h"

namespace watery {

Data::Data(DataDescriptor descriptor) : _descriptor{descriptor} {}

std::unique_ptr<Data> Data::decode(DataDescriptor descriptor, const uint8_t *buffer) {
    return std::unique_ptr<Data>();
}

const DataDescriptor &Data::descriptor() const {
    return _descriptor;
}

}