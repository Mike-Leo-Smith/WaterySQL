//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_CONFIG_H
#define WATERYSQL_CONFIG_H

#include <cstdint>
#include <string_view>

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

static constexpr uint32_t PAGE_SIZE = 16384;  // 16K
static constexpr uint32_t MAX_FILE_COUNT = 31;
static constexpr uint32_t MAX_BUFFERED_PAGE_COUNT = 65535;

static constexpr uint32_t MAX_IDENTIFIER_LENGTH = 32;
static constexpr uint32_t MAX_FIELD_COUNT = 32;

static constexpr uint32_t MAX_SLOT_COUNT_PER_PAGE = 512;

using FieldNullBitset = BitsetHolder<MAX_FIELD_COUNT>;

static constexpr auto DATABASE_BASE_PATH = "watery-db";
static constexpr auto DATABASE_DIRECTORY_EXTENSION = ".db";

static constexpr auto TABLE_FILE_EXTENSION = ".tab";
static constexpr auto INDEX_FILE_EXTENSION = ".idx";

}

}

#endif  // WATERYSQL_CONFIG_H
