//
// Created by Mike Smith on 2018-12-09.
//

#ifndef WATERYSQL_FIELD_CONSTRAINT_H
#define WATERYSQL_FIELD_CONSTRAINT_H

#include <bitset>

namespace watery {

class FieldConstraint final {

public:
    static constexpr uint8_t UNIQUE_BIT_MASK = 1 << 0;
    static constexpr uint8_t INDEXED_BIT_MASK = 1 << 1;
    static constexpr uint8_t NULLABLE_BIT_MASK = 1 << 2;
    static constexpr uint8_t FOREIGN_BIT_MASK = 1 << 3;
    static constexpr uint8_t PRIMARY_BIT_MASK = 1 << 4;

private:
    uint8_t _constraints{0};

public:
    FieldConstraint() = default;
    explicit FieldConstraint(uint8_t constraints) : _constraints{static_cast<uint8_t>(constraints)} {}
    
    void set(uint8_t mask, bool val) noexcept { val ? (_constraints |= mask) : (_constraints &= ~mask); }
    constexpr bool get(uint8_t mask) const noexcept { return (_constraints & mask) != 0; }
    
    constexpr bool unique() const noexcept { return get(UNIQUE_BIT_MASK); }
    constexpr bool indexed() const noexcept { return get(INDEXED_BIT_MASK); }
    constexpr bool nullable() const noexcept { return get(NULLABLE_BIT_MASK); }
    constexpr bool foreign() const noexcept { return get(FOREIGN_BIT_MASK); }
    constexpr bool primary() const noexcept { return get(PRIMARY_BIT_MASK); }
    
    void set_unique(bool val) noexcept { set(UNIQUE_BIT_MASK, val); }
    void set_indexed(bool val) noexcept { set(INDEXED_BIT_MASK, val); }
    void set_nullable(bool val) noexcept { set(NULLABLE_BIT_MASK, val); }
    void set_foreign(bool val) noexcept { set(FOREIGN_BIT_MASK, val); }
    void set_primary(bool val) noexcept {
        set(val ?
            PRIMARY_BIT_MASK | UNIQUE_BIT_MASK:  // primary keys are always unique
            PRIMARY_BIT_MASK, val);
    }
};

}

#endif  // WATERYSQL_FIELD_CONSTRAINT_H
