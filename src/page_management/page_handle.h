//
// Created by Mike Smith on 2018-12-22.
//

#ifndef WATERYSQL_PAGE_HANDLE_H
#define WATERYSQL_PAGE_HANDLE_H

#include "../config/config.h"

namespace watery {

struct PageHandle {
    FileHandle file_handle{-1};
    PageOffset page_offset{-1};
    
    bool operator==(PageHandle rhs) const noexcept {
        return file_handle == rhs.file_handle && page_offset == rhs.page_offset;
    }
    
    struct Hash {
        uint64_t operator()(PageHandle h) const noexcept {
            return std::hash<uint64_t>{}((static_cast<uint64_t>(h.file_handle) << 32) | h.page_offset);
        }
    };
    
};

}

#endif  // WATERYSQL_PAGE_HANDLE_H
