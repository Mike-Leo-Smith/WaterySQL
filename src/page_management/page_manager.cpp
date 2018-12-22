//
// Created by Mike Smith on 2018/11/24.
//

#include <filesystem>

#include "page_manager.h"
#include "../errors/page_manager_error.h"

namespace watery {

Byte *FileManager::_get_cache(CacheHandle h) noexcept {
    return _cache.data() + h * PAGE_SIZE;
}

Byte *FileManager::_access_cache(CacheHandle h) {
    _cache_usage.erase(_cache_descriptors[h].time_stamp);
    _cache_usage.emplace(_cache_descriptors[h].time_stamp = _assign_time_stamp(), h);
    return _get_cache(h);
}

CacheHandle FileManager::_assign_cache_handle(PageHandle page_handle) {
    
    // no free page remains, flush the least recently used one.
    if (_available_cache_handles.empty()) {
        _flush_cache(_cache_usage.begin()->second);
    }
    
    // get a free cache handle
    auto cache_handle = _available_cache_handles.front();
    _available_cache_handles.pop();
    auto time_stamp = _assign_time_stamp();
    _cache_usage.emplace(time_stamp, cache_handle);
    _cache_descriptors[cache_handle].time_stamp = time_stamp;
    _cache_descriptors[cache_handle].page_handle = page_handle;
    _cached_pages.emplace(page_handle, cache_handle);
    _file_associated_cache_sets[page_handle.file_handle].emplace(cache_handle);
    
    return cache_handle;
}

inline TimeStamp FileManager::_assign_time_stamp() noexcept {
    return _current_time_stamp++;
}

void FileManager::_flush_cache(CacheHandle h) {
    auto &cache_desc = _cache_descriptors[h];
    auto page_handle = cache_desc.page_handle;
    if (cache_desc.dirty) {  // write back if the page is dirty
        auto &file = _open_files[page_handle.file_handle];
        if (!file.is_open()) {
            throw PageManagerError{"Cannot write back the cache into a closed file."};
        }
        file.seekp(cache_desc.page_handle.page_offset * PAGE_SIZE, std::ios::beg);
        file.write(_get_cache(h), PAGE_SIZE);
    }
    cache_desc.dirty = false;
    _cached_pages.erase(cache_desc.page_handle);
    _cache_usage.erase(cache_desc.time_stamp);
    _available_cache_handles.emplace(h);
    _file_associated_cache_sets[page_handle.file_handle].erase(h);
}

FileManager::FileManager() {
    std::ios::sync_with_stdio(false);
    _cache.resize(PAGE_SIZE * MAX_PAGE_COUNT);
    for (auto i = 0u; i < MAX_PAGE_COUNT; i++) {
        _available_cache_handles.emplace(i);
    }
    for (auto i = 0; i < MAX_FILE_COUNT; i++) {
        _available_file_handles.emplace(i);
    }
    _cache_descriptors.resize(MAX_PAGE_COUNT);
    _open_files.resize(MAX_FILE_COUNT);
    _file_associated_cache_sets.resize(MAX_FILE_COUNT);
}

FileManager::~FileManager() {
    for (auto fh = 0; fh < MAX_FILE_COUNT; fh++) {
        close_file(fh);
    }
}

FileHandle FileManager::open_file(const std::string &file_name) {
    if (_available_file_handles.empty()) {
        throw PageManagerError{"Too many files to open."};
    }
    auto handle = _available_file_handles.front();
    _available_file_handles.pop();
    _open_files[handle] = std::fstream{file_name, std::ios::binary | std::ios::in | std::ios::out};
    if (!_open_files[handle].is_open()) {
        throw PageManagerError{std::string{"Failed to open file "}.append(file_name).append("\".")};
    }
    return handle;
}

void FileManager::close_file(FileHandle h) {
    
    // flush all the cached pages associated to the file.
    while (!_file_associated_cache_sets[h].empty()) {
        // note that we cannot use a for-loop here since method `_flush_cache` will erase the
        // cache handle from the set and therefore leading to the corruption of the iterator.
        _flush_cache(*_file_associated_cache_sets[h].begin());
    }
    
    // now the file can be closed and the file handle can be recycled.
    _open_files[h].close();
    _available_file_handles.emplace(h);
}

void FileManager::create_file(const std::string &file_name) {
    std::ofstream{file_name};  // create the file
    if (!std::filesystem::exists(file_name)) {  // check if successful
        throw PageManagerError{std::string{"Failed to create file\""}.append(file_name).append("\"")};
    }
}

void FileManager::delete_file(const std::string &file_name) {
    if (!std::filesystem::remove(file_name)) {
        throw PageManagerError(std::string{"Failed to delete file \""}.append(file_name).append("\"."));
    }
}

CacheHandle FileManager::allocate_page_cache(PageHandle page_handle) {
    if (auto it = _cached_pages.find(page_handle); it != _cached_pages.end()) {  // already cached
        return it->second;
    }
    return _assign_cache_handle(page_handle);
}

CacheHandle FileManager::load_page_cache(PageHandle page_handle) {
    
    // return the cache handle if the page is already cached.
    if (auto it = _cached_pages.find(page_handle); it != _cached_pages.end()) {  // already cached
        return it->second;
    }
    
    // not cached, assign a new one.
    auto cache_handle = _assign_cache_handle(page_handle);
    auto cache = _get_cache(cache_handle);
    
    // load the contents of the page into the cache.
    auto &file = _open_files[page_handle.file_handle];
    if (!file.is_open()) {
        throw PageManagerError{"Failed to get cache handle from a closed file."};
    }
    file.seekg(page_handle.page_offset * PAGE_SIZE, std::ios::beg);
    file.read(cache, PAGE_SIZE);
    
    return cache_handle;
}

Byte *FileManager::access_cache_for_writing(CacheHandle h) {
    _cache_descriptors[h].dirty = true;
    return _access_cache(h);
}

const Byte *FileManager::access_cache_for_reading(CacheHandle h) {
    return _access_cache(h);
}

}
