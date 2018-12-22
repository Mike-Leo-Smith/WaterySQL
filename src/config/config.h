//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_CONFIG_H
#define WATERYSQL_CONFIG_H

#include <cstdint>
#include <string_view>
#include <array>
#include <bitset>

namespace watery {

inline namespace config {

using TimeStamp = uint32_t;
using FileHandle = int32_t;
using PageOffset = int32_t;
using CacheHandle = int32_t;

using PageOffset = int32_t;
using SlotOffset = int32_t;
using ChildOffset = int32_t;

using Byte = char;

static constexpr uint32_t MAX_PAGE_COUNT = 32;
static constexpr uint32_t PAGE_SIZE = 16384;  // 16K
static constexpr uint32_t MAX_FILE_COUNT = 31;
static constexpr uint32_t MAX_BUFFERED_PAGE_COUNT = 65535;

static constexpr uint32_t MAX_IDENTIFIER_LENGTH = 31;
static constexpr uint32_t MAX_FIELD_COUNT = 32;

static constexpr uint32_t MAX_SLOT_COUNT_PER_PAGE = 512;

static constexpr auto DATABASE_BASE_PATH = "watery-db";
static constexpr auto DATABASE_DIRECTORY_EXTENSION = ".db";
static constexpr auto TABLE_FILE_EXTENSION = ".tab";
static constexpr auto INDEX_FILE_EXTENSION = ".idx";

using Identifier = std::array<Byte, MAX_IDENTIFIER_LENGTH + 1>;
using NullFieldBitmap = std::bitset<MAX_FIELD_COUNT>;

}

}

#endif  // WATERYSQL_CONFIG_H
