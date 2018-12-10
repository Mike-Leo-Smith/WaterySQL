//
// Created by Mike Smith on 2018-12-09.
//

#ifndef WATERYSQL_FIELD_CONSTRAINT_H
#define WATERYSQL_FIELD_CONSTRAINT_H

#include <bitset>

namespace watery {

class FieldConstraint final {

public:
    static constexpr auto UNIQUE_BIT_MASK = 0x01u;
    static constexpr auto INDEXED_BIT_MASK = 0x02u;

private:
    uint8_t _constraints{0};

public:
    explicit FieldConstraint(uint32_t constraints = 0u) : _constraints{static_cast<uint8_t>(constraints)} {}
    void set(uint32_t mask, bool val) noexcept { val ? (_constraints |= mask) : (_constraints &= ~mask); }
    constexpr bool get(int mask) const noexcept { return (_constraints & mask) != 0; }
    constexpr bool unique() const noexcept { return get(UNIQUE_BIT_MASK); }
    void set_unique(bool val) noexcept { set(UNIQUE_BIT_MASK, val); }
    constexpr bool indexed() const noexcept { return get(INDEXED_BIT_MASK); }
    void set_indexed(bool val) noexcept { set(INDEXED_BIT_MASK, val); }
    
};

}

#endif  // WATERYSQL_FIELD_CONSTRAINT_H
