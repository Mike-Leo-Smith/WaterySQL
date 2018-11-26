#ifndef PAGE_DEF
#define PAGE_DEF

#include <cstdint>

static constexpr uint32_t PAGE_SIZE = 8192;
static constexpr uint32_t PAGE_SIZE_IDX = 13;
static constexpr uint32_t MAX_FILE_NUM = 128;
static constexpr uint32_t MAX_BUFFERED_PAGE_COUNT = 60000;

typedef uint8_t* Buffer;

#endif
