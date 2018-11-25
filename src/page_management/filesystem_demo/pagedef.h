#ifndef PAGE_DEF
#define PAGE_DEF
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

static constexpr uint32_t PAGE_SIZE = 8192;
static constexpr uint32_t PAGE_SIZE_IDX = 13;
static constexpr uint32_t MAX_FILE_NUM = 128;
static constexpr uint32_t MAX_BUFFERED_PAGE_COUNT = 60000;

typedef unsigned int* BufType;

#endif
