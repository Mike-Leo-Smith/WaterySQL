//
// Created by Mike Smith on 2018/11/25.
//

#include "data.h"
#include "integer.h"
#include "float.h"
#include "varchar.h"
#include "../errors/data_error.h"

namespace watery {

std::unique_ptr<Data> Data::decode(DataDescriptor descriptor, const Byte *raw) {
    switch (descriptor.type) {
    case TypeTag::INTEGER:
        return std::make_unique<Integer>(MemoryMapper::map_memory<int32_t>(raw));
    case TypeTag::FLOAT:
        return std::make_unique<Float>(MemoryMapper::map_memory<float>(raw));
    case TypeTag::VARCHAR:
        return std::make_unique<Varchar>(reinterpret_cast<const char *>(raw), descriptor.size_hint);
    default:
        throw DataError{"Failed to decode data with unknown type."};
    }
}

}

