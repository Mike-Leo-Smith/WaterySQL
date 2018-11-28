//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_DATA_TYPE_H
#define WATERYSQL_DATA_TYPE_H

#include <cstdint>
#include <memory>

#include "../config/config.h"
#include "type_tag.h"
#include "data_descriptor.h"
#include "../utility/type_constraints/non_copyable.h"
#include "../utility/memory_mapping/memory_mapper.h"
#include "../errors/data_error.h"

namespace watery {

struct Data : NonCopyable {
    virtual ~Data() = default;
    virtual TypeTag type() const = 0;
    virtual uint32_t length() const = 0;
    static std::unique_ptr<Data> decode(DataDescriptor descriptor, const Byte *raw);
    virtual void encode(Byte *buffer) const = 0;
    virtual std::unique_ptr<Data> replica() const = 0;
    
    static bool less(DataDescriptor d, const Byte *lhs, const Byte *rhs) {
        switch (d.type) {
        case TypeTag::INTEGER:
            return MemoryMapper::map_memory<int32_t>(lhs) < MemoryMapper::map_memory<int32_t>(rhs);
        case TypeTag::FLOAT:
            return MemoryMapper::map_memory<float>(lhs) < MemoryMapper::map_memory<float>(rhs);
        case TypeTag::VARCHAR:
            return std::string_view{reinterpret_cast<const char *>(lhs), d.length} <
                   std::string_view{reinterpret_cast<const char *>(rhs), d.length};
        default:
            throw DataError{"Failed to compare data with unsupported types."};
        }
    }
    
    virtual bool operator<(const Data &rhs) const {
        throw DataError{"Failed to compare data due to unsupported operator [<]."};
    }
    
    virtual bool operator<=(const Data &rhs) const {
        throw DataError{"Failed to compare data due to unsupported operator [<=]."};
    }
    
    virtual bool operator==(const Data &rhs) const {
        throw DataError{"Failed to compare data due to unsupported operator [==]."};
    }
    
    virtual bool operator>=(const Data &rhs) const {
        throw DataError{"Failed to compare data due to unsupported operator [>=]."};
    }
    
    virtual bool operator>(const Data &rhs) const {
        throw DataError{"Failed to compare data due to unsupported operator [>]."};
    }
};

}

#endif  // WATERYSQL_DATA_TYPE_H
