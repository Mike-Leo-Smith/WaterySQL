//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_CONFIG_H
#define WATERYSQL_CONFIG_H

#include <cstdint>
#include "../utility/type_manipulators/bitset_holder_type_selector.h"

namespace watery {

inline namespace config {

using FileHandle = int32_t;
using BufferHandle = int32_t;

using PageOffset = int32_t;
using SlotOffset = int32_t;
using ChildOffset = int32_t;

using Byte = uint8_t;
using Buffer = Byte *;

static constexpr uint32_t PAGE_SIZE = 1 << 14;  // 16K
static constexpr uint32_t MAX_FILE_COUNT = 31;
static constexpr uint32_t MAX_BUFFERED_PAGE_COUNT = (1 << 18) - 1;

static constexpr uint32_t MAX_FIELD_NAME_LENGTH = 32;
static constexpr uint32_t MAX_FIELD_COUNT = 32;

static constexpr uint32_t MAX_SLOT_COUNT_PER_PAGE = 512;

using FieldNullBitset = BitsetHolder<MAX_FIELD_COUNT>;

}

}

#endif  // WATERYSQL_CONFIG_H
