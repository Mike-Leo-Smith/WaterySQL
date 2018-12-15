//
// Created by Mike Smith on 2018-12-09.
//

#ifndef WATERYSQL_DATA_COMPARATOR_H
#define WATERYSQL_DATA_COMPARATOR_H

#include "data_descriptor.h"
#include "../config/config.h"
#include "../utility/memory/memory_mapper.h"
#include "../utility/mathematics/sgn.h"

namespace watery {

class DataComparator final {

private:
    DataDescriptor _descriptor;

public:
    DataComparator() = default;
    
    explicit DataComparator(DataDescriptor desc) : _descriptor{desc} {}
    
    int32_t compare(const Byte *lhs, const Byte *rhs) const noexcept {
        switch (_descriptor.type) {
            case TypeTag::INTEGER:
            case TypeTag::DATE:
                return MemoryMapper::map_memory<int32_t>(lhs) - MemoryMapper::map_memory<int32_t>(rhs);
            case TypeTag::FLOAT:
                return sgn(MemoryMapper::map_memory<float>(lhs) - MemoryMapper::map_memory<float>(rhs));
            case TypeTag::CHAR:
                return std::string_view{reinterpret_cast<const char *>(lhs)}
                    .compare(std::string_view{reinterpret_cast<const char *>(lhs)});
            default:
                return 0;
        }
    }
    
    bool equal(const Byte *lhs, const Byte *rhs) const noexcept { return compare(lhs, rhs) == 0; }
    bool less(const Byte *lhs, const Byte *rhs) const noexcept { return compare(lhs, rhs) < 0; }
    bool greater(const Byte *lhs, const Byte *rhs) const noexcept { return compare(lhs, rhs) > 0; }
    bool less_equal(const Byte *lhs, const Byte *rhs) const noexcept { return compare(lhs, rhs) <= 0; }
    bool greater_equal(const Byte *lhs, const Byte *rhs) const noexcept { return compare(lhs, rhs) >= 0; }
    bool unequal(const Byte *lhs, const Byte *rhs) const noexcept { return compare(lhs, rhs) != 0; }
    
};

}

#endif  // WATERYSQL_DATA_COMPARATOR_H
