//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_PAGE_MANAGER_H
#define WATERYSQL_PAGE_MANAGER_H

#include "../utility/type/singleton.h"
#include "filesystem_demo/FileManager.h"
#include "filesystem_demo/BufferedPageManager.h"
#include "buffered_page.h"

namespace watery {

class PageManager : public Singleton<PageManager> {

private:
    FileManager _file_manager{};
    BufferedPageManager _buffer_manager{_file_manager};

protected:
    PageManager() = default;
    ~PageManager() = default;

public:
    bool file_exists(const std::string &file_name) const;
    void create_file(const std::string &file_name);
    void delete_file(const std::string &file_name);
    FileHandle open_file(const std::string &file_name);
    void close_file(FileHandle file_handle) noexcept;
    PageHandle get_page(FileHandle file_handle, PageOffset page_offset) noexcept;
    PageHandle allocate_page(FileHandle file_handle, PageOffset page_offset) noexcept;
    void mark_page_dirty(PageHandle page) noexcept;
    void flush_page(PageHandle page) noexcept;
    void mark_page_accessed(PageHandle page) noexcept;
    bool not_flushed(PageHandle page) noexcept;
};

}

#endif  // WATERYSQL_PAGE_MANAGER_H
