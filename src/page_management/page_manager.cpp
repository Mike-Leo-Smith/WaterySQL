//
// Created by Mike Smith on 2018/11/24.
//

#include <filesystem>
#include "page_manager.h"
#include "../errors/page_manager_error.h"

namespace watery {

bool PageManager::file_exists(const std::string &file_name) const {
    return std::filesystem::exists(file_name);
}

void PageManager::create_file(const std::string &file_name) {
    if (!_file_manager.create_file(file_name.c_str())) {
        throw PageManagerError{std::string{"Failed to create file \""}.append(file_name).append("\".")};
    }
}

FileHandle PageManager::open_file(const std::string &file_name) {
    auto file_id = 0;
    if (!_file_manager.open_file(file_name.c_str(), file_id)) {
        throw PageManagerError{std::string{"Failed to open file \""}.append(file_name).append("\".")};
    }
    return file_id;
}

void PageManager::close_file(FileHandle file_handle) noexcept {
    _file_manager.close_file(file_handle);
}

PageHandle PageManager::get_page(FileHandle file_handle, PageOffset page_offset) noexcept {
    BufferHandle buffer_handle;
    auto buffer = _buffer_manager.get_page(file_handle, page_offset, buffer_handle);
    return {file_handle, page_offset, buffer_handle, buffer};
}

PageHandle PageManager::allocate_page(FileHandle file_handle, PageOffset page_offset) noexcept {
    BufferHandle buffer_handle;
    auto buffer = _buffer_manager.allocate_page(file_handle, page_offset, buffer_handle);
    return {file_handle, page_offset, buffer_handle, buffer};
}

void PageManager::mark_page_dirty(PageHandle page) noexcept {
    _buffer_manager.mark_dirty(page.buffer_handle);
}

void PageManager::flush_page(PageHandle page) noexcept {
    _buffer_manager.write_back(page.buffer_handle);
}

void PageManager::mark_page_accessed(PageHandle page) noexcept {
    _buffer_manager.mark_access(page.buffer_handle);
}

bool PageManager::not_flushed(PageHandle page) noexcept {
    FileHandle file_handle;
    PageOffset page_offset;
    _buffer_manager.get_key(page.buffer_handle, file_handle, page_offset);
    return file_handle == page.buffer_offset.file_handle &&
           page_offset == page.buffer_offset.page_offset;
}

void PageManager::delete_file(const std::string &file_name) {
    if (!std::filesystem::remove(file_name)) {
        throw PageManagerError(std::string{"Failed to delete file \""}.append(file_name).append("\"."));
    }
}

}
