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
    FileHandle open(const std::string &file_name);
    void close(FileHandle file_handle) noexcept;
    Page get_page(FileHandle file_handle, PageOffset page_offset);
    Page allocate_page(FileHandle file_handle, PageOffset page_offset);
    void mark_page_dirty(PageHandle page_handle);
    void write_page_back(PageHandle page_handle);
    void mark_page_accessed(PageHandle page_handle);
    bool is_replaced(Page page);
};

}

#endif  // WATERYSQL_PAGE_MANAGER_H
