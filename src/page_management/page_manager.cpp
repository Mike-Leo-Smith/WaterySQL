//
// Created by Mike Smith on 2018/11/24.
//

#include <filesystem>
#include "page_manager.h"
#include "../errors/file_io_error.h"

namespace watery {

bool PageManager::file_exists(const std::string &file_name) const {
    return std::filesystem::exists(file_name);
}

void PageManager::create_file(const std::string &file_name) {
    if (!_file_manager.create_file(file_name.c_str())) {
        throw FileIOError{std::string{"Failed to create file \""}.append(file_name).append("\".")};
    }
}

FileHandle PageManager::open(const std::string &file_name) {
    auto file_id = 0;
    if (!_file_manager.open_file(file_name.c_str(), file_id)) {
        throw FileIOError{std::string{"Failed to open file \""}.append(file_name).append("\".")};
    }
    return file_id;
}

void PageManager::close(FileHandle file_handle) noexcept {
    _file_manager.close_file(file_handle);
}

Page PageManager::get_page(FileHandle file_handle, PageOffset page_offset) {
    PageHandle page_handle;
    auto buffer = reinterpret_cast<Byte *>(_buffer_manager.get_page(file_handle, page_offset, page_handle));
    return {file_handle, page_offset, page_handle, buffer};
}

Page PageManager::allocate_page(FileHandle file_handle, PageOffset page_offset) {
    PageHandle page_handle;
    auto buffer = reinterpret_cast<Byte *>(_buffer_manager.allocate_page(file_handle, page_offset, page_handle));
    return {file_handle, page_offset, page_handle, buffer};
}

void PageManager::mark_page_dirty(PageHandle page_handle) {
    _buffer_manager.mark_dirty(page_handle);
}

void PageManager::write_page_back(PageHandle page_handle) {
    _buffer_manager.write_back(page_handle);
}

void PageManager::mark_page_accessed(PageHandle page_handle) {
    _buffer_manager.mark_access(page_handle);
}

bool PageManager::is_replaced(Page page) {
    FileHandle file_handle;
    PageOffset page_offset;
    _buffer_manager.get_key(page.page_handle, file_handle, page_offset);
    return file_handle == page.file_handle &&
           page_offset == page.page_offset;
}

PageManager::PageManager() {
    MyBitMap::initConst();   //新加的初始化
}

}
