//
// Created by Mike Smith on 2018-12-09.
//

#ifndef WATERYSQL_FIELD_CONSTRAINT_H
#define WATERYSQL_FIELD_CONSTRAINT_H

#include <bitset>

namespace watery {

class FieldConstraint final {

public:
    
    using Mask = uint8_t;
    
    static constexpr Mask UNIQUE_BIT_MASK = 1 << 0;
    static constexpr Mask NULLABLE_BIT_MASK = 1 << 1;
    static constexpr Mask FOREIGN_BIT_MASK = 1 << 2;
    static constexpr Mask PRIMARY_BIT_MASK = 1 << 3;

private:
    Mask _constraints{0};

public:
    FieldConstraint() = default;
    explicit FieldConstraint(Mask constraints) : _constraints{static_cast<Mask>(constraints)} {}
    
    constexpr bool unique() const noexcept { return _constraints & UNIQUE_BIT_MASK; }
    constexpr bool nullable() const noexcept { return _constraints & NULLABLE_BIT_MASK; }
    constexpr bool foreign() const noexcept { return _constraints & FOREIGN_BIT_MASK; }
    constexpr bool primary() const noexcept { return _constraints & PRIMARY_BIT_MASK; }
    
    void set_unique() noexcept { _constraints |= UNIQUE_BIT_MASK; }
    void set_nullable() noexcept { _constraints |= NULLABLE_BIT_MASK; }
    void set_foreign() noexcept { _constraints |= FOREIGN_BIT_MASK; }
    void set_primary() noexcept { _constraints |= PRIMARY_BIT_MASK | UNIQUE_BIT_MASK; }
    
};

}

#endif  // WATERYSQL_FIELD_CONSTRAINT_H
