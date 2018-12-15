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
#include "../errors/data_error.h"

namespace watery {

struct DataView {
    
    DataDescriptor descriptor;
    std::variant<int32_t, float, std::string_view> buffer;
    
    DataView(DataDescriptor desc, Byte *buf) : descriptor{desc} {
        switch (descriptor.type) {
            case TypeTag::INTEGER:
            case TypeTag::DATE:
                buffer = MemoryMapper::map_memory<int32_t>(buf);
                break;
            case TypeTag::FLOAT:
                buffer = MemoryMapper::map_memory<float>(buf);
                break;
            case TypeTag::CHAR:
                buffer = std::string_view{reinterpret_cast<char *>(buf)};
                break;
            default:
                throw DataError{"Unknown type tag."};
        }
    }
    
    DataView(DataDescriptor desc, std::string_view raw) : descriptor{desc} {
        
    }
    
    void encode(Byte *buffer) const noexcept {
    
    }
    
};

}

#endif  // WATERYSQL_DATA_VIEW_H
