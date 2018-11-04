//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_CONFIG_H
#define WATERYSQL_CONFIG_H

#include <cstdint>

namespace watery {
inline namespace config {

static constexpr uint32_t SOME_VALUE = 10;

static constexpr uint32_t MAX_FIELD_NAME_LENGTH = 32;
static constexpr uint32_t MAX_FIELD_COUNT = 32;

static constexpr uint32_t MAX_RECORD_COUNT_PER_PAGE = 256;

}
}

#endif  // WATERYSQL_CONFIG_H
