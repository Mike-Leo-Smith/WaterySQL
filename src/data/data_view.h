//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_DATA_VIEW_H
#define WATERYSQL_DATA_VIEW_H

#include <variant>

#include "type_tag.h"
#include "../config/config.h"
#include "data_descriptor.h"

#include "../utility/memory/memory_mapper.h"
#include "../error/data_error.h"
#include "../utility/memory/string_view_copier.h"
#include "../utility/memory/value_decoder.h"

namespace watery {

struct DataView {
    
    DataDescriptor descriptor;
    std::variant<int32_t, float, std::string_view> holder;
    
    DataView(DataDescriptor desc, Byte *buf) : descriptor{desc} {
        switch (descriptor.type) {
            case TypeTag::INTEGER:
            case TypeTag::DATE:
                holder = MemoryMapper::map_memory<int32_t>(buf);
                break;
            case TypeTag::FLOAT:
                holder = MemoryMapper::map_memory<float>(buf);
                break;
            case TypeTag::CHAR:
                holder = std::string_view{buf};
                break;
            default:
                throw DataError{"Unknown type tag."};
        }
    }
    
    DataView(DataDescriptor desc, std::string_view raw) : descriptor{desc} {
        switch (descriptor.type) {
            case TypeTag::INTEGER:
            case TypeTag::DATE:
                holder = ValueDecoder::decode_integer(raw);
                break;
            case TypeTag::FLOAT:
                holder = ValueDecoder::decode_float(raw);
                break;
            case TypeTag::CHAR:
                holder = raw;
                break;
            default:
                throw DataError{"Unknown type tag."};
        }
    }
    
    void encode(Byte *buffer) const noexcept {
        switch (descriptor.type) {
            case TypeTag::DATE:
            case TypeTag::INTEGER:
                MemoryMapper::map_memory<int32_t>(buffer) = std::get<int32_t>(holder);
                break;
            case TypeTag::FLOAT:
                MemoryMapper::map_memory<float>(buffer) = std::get<int32_t>(holder);
                break;
            case TypeTag::CHAR:
                StringViewCopier::copy(std::get<std::string_view>(holder), buffer);
                break;
            default:
                throw DataError{"Unknown type tag."};
        }
    }
    
};

}

#endif  // WATERYSQL_DATA_VIEW_H
