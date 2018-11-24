//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_PAGE_MANAGER_H
#define WATERYSQL_PAGE_MANAGER_H

#include "../utility/type_constraints/singleton.h"
#include "../filesystem_demo/fileio/FileManager.h"
#include "../filesystem_demo/bufmanager/BufferedPageManager.h"
#include "page.h"

namespace watery {

class PageManager : public Singleton<PageManager> {

private:
    FileManager _file_manager{};
    BufferedPageManager _buffer_manager{&_file_manager};

protected:
    PageManager();

public:
    bool file_exists(const std::string &file_name) const;
    void create_file(const std::string &file_name);
    void delete_file(const std::string &file_name);
    FileHandle open_file(const std::string &file_name);
    void close_file(FileHandle file_handle) noexcept;
    Page get_page(FileHandle file_handle, PageOffset page_offset) noexcept;
    Page allocate_page(FileHandle file_handle, PageOffset page_offset) noexcept;
    void mark_page_dirty(Page page) noexcept;
    void flush_page(Page page) noexcept;
    void mark_page_accessed(Page page) noexcept;
    bool is_replaced(Page page) noexcept;
};

}

#endif  // WATERYSQL_PAGE_MANAGER_H
