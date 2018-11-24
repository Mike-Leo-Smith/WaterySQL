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
using PageHandle = int32_t;

using PageOffset = int32_t;
using SlotOffset = int32_t;

using Byte = uint8_t;

static constexpr uint32_t MAX_TABLE_NAME_LENGTH = 64;
static constexpr uint32_t MAX_FIELD_NAME_LENGTH = 32;
static constexpr uint32_t MAX_FIELD_COUNT = 32;

static constexpr uint32_t MAX_RECORD_COUNT_PER_PAGE = 256;
static constexpr uint32_t SLOT_BITSET_SIZE = (MAX_RECORD_COUNT_PER_PAGE + 7) / 8;
static constexpr uint32_t MAX_DATA_SIZE = 32767;

static constexpr uint32_t MAX_BTREE_NODE_POINTER_COUNT = 32;
static constexpr uint32_t MAX_BTREE_NODE_KEY_COUNT = MAX_BTREE_NODE_POINTER_COUNT - 1;
static constexpr uint32_t MIN_BTREE_NODE_POINTER_COUNT = (MAX_BTREE_NODE_POINTER_COUNT + 1) / 2;
static constexpr uint32_t MIN_BTREE_NODE_KEY_COUNT = MIN_BTREE_NODE_POINTER_COUNT - 1;

using FieldNullBitset = BitsetHolder<MAX_FIELD_COUNT>;

}

}

#endif  // WATERYSQL_CONFIG_H
