//
// Created by Mike Smith on 2018-12-09.
//

#ifndef WATERYSQL_FIELD_CONSTRAINT_H
#define WATERYSQL_FIELD_CONSTRAINT_H

#include <bitset>

namespace watery {

class FieldConstraint final {

public:
    static constexpr auto UNIQUE_BIT_INDEX = 0u;
    static constexpr auto NULLABLE_BIT_INDEX = 1u;

private:
    uint8_t _constraints{0};

public:
    void set(int constraint, bool val) noexcept { _constraints |= (1u << constraint); }
    constexpr bool get(int constraint) const noexcept { return (_constraints & (1u << constraint)) != 0; }
    constexpr bool unique() const noexcept { return get(UNIQUE_BIT_INDEX); }
    void set_unique(bool val) noexcept { set(UNIQUE_BIT_INDEX, val); }
    constexpr bool nullable() const noexcept { return get(NULLABLE_BIT_INDEX); }
    void set_nullable(bool val) noexcept { set(NULLABLE_BIT_INDEX, val); }
    
};

}

#endif  // WATERYSQL_FIELD_CONSTRAINT_H
