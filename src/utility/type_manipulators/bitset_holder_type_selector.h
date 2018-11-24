//
// Created by Mike Smith on 2018/11/23.
//

#ifndef WATERYSQL_BITSET_HOLDER_TYPE_SELECTOR_H
#define WATERYSQL_BITSET_HOLDER_TYPE_SELECTOR_H

#include <cstdint>

namespace watery {

namespace _impl {

template<uint64_t byte_count>
struct BitsetHolderTypeSelectorImpl {
    static_assert((byte_count > 0) && (byte_count <= 8), "Given bits cannot be capsuled in a single integer");
};

template<>
struct BitsetHolderTypeSelectorImpl<1> {
    using Type = uint8_t;
};

template<>
struct BitsetHolderTypeSelectorImpl<2> {
    using Type = uint16_t;
};

template<>
struct BitsetHolderTypeSelectorImpl<4> {
    using Type = uint32_t;
};

template<>
struct BitsetHolderTypeSelectorImpl<8> {
    using Type = uint64_t;
};

}

template<uint64_t bit_count>
using BitsetHolder = typename _impl::BitsetHolderTypeSelectorImpl<(bit_count + 7) / 8>::Type;

}



#endif  // WATERYSQL_BITSET_HOLDER_TYPE_SELECTOR_H
