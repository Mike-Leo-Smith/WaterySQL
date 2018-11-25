//
// Created by Mike Smith on 2018/11/24.
//

#include "index_manager.h"
#include "index_node_pointer_offset.h"
#include "../errors/index_manager_error.h"
#include "../errors/page_manager_error.h"
#include "../utility/io_helpers/error_printer.h"
#include "../utility/memory_mapping/memory_mapper.h"

namespace watery {

void IndexManager::create_index(const std::string &name, DataDescriptor key_descriptor) {
    auto kl = key_descriptor.size;
    auto pl = static_cast<uint32_t>(sizeof(IndexNodePointerOffset));
    auto cpn = std::min(MAX_CHILD_COUNT_PER_INDEX_NODE,
                        static_cast<uint32_t>((PAGE_SIZE - sizeof(IndexNodeHeader)) / (kl + pl)));
    
    if (cpn == 0) {
        throw IndexManagerError{std::string{"Failed to create index because the keys are too long ("}
                                    .append(std::to_string(kl)).append(" bytes).")};
    }
    if (_page_manager.file_exists(name)) {
        throw IndexManagerError{
            std::string{"Failed to create file for index \""}.append(name).append("\" which already exists.")};
    }
    
    FileHandle file_handle;
    try {
        _page_manager.create_file(name);
        file_handle = _page_manager.open_file(name);
    } catch (const PageManagerError &e) {
        print_error(std::cerr, e);
        throw IndexManagerError{std::string{"Failed to create file for index \""}.append(name).append("\".")};
    }
    
    auto page = _page_manager.allocate_page(file_handle, 0);
    
    MemoryMapper::map_memory<IndexHeader>(page.data) = {key_descriptor, 1, kl, pl, cpn};
    _page_manager.mark_page_dirty(page);
    _page_manager.flush_page(page);
    _page_manager.close_file(file_handle);
}

void IndexManager::delete_index(const std::string &name) {
    if (is_index_open(name)) {
        throw IndexManagerError{std::string{"Failed to delete index \""}.append(name).append("\" which is in use.")};
    }
    try {
        _page_manager.delete_file(name);
    } catch (const PageManagerError &e) {
        print_error(std::cerr, e);
        throw IndexManagerError{std::string{"Failed to delete the file for index \""}.append(name).append("\".")};
    }
}

Index IndexManager::open_index(const std::string &name) {
    if (is_index_open(name)) {
        throw IndexManagerError{std::string{"Failed to open index \""}.append(name).append("\" that is already open.")};
    }
    FileHandle file_handle;
    try {
        file_handle = _page_manager.open_file(name);
    } catch (const PageManagerError &e) {
        print_error(std::cerr, e);
        throw IndexManagerError(std::string{"Failed to open file for table \""}.append(name).append("\"."));
    }
    _used_buffers.emplace(name, std::unordered_map<BufferHandle, BufferOffset>{});
    
    // load table header
    auto header_page = _page_manager.get_page(file_handle, 0);
    _page_manager.mark_page_accessed(header_page);
    auto index_header = MemoryMapper::map_memory<IndexHeader>(header_page.data);
    
    return {name, file_handle, index_header};
}

void IndexManager::close_index(const Index &index) {
    if (!is_index_open(index.name)) {
        return;
    }
    // update table header
    auto header_page = _page_manager.get_page(index.file_handle, 0);
    MemoryMapper::map_memory<IndexHeader>(header_page.data) = index.header;
    _page_manager.mark_page_dirty(header_page);
    _page_manager.flush_page(header_page);
    
    for (auto &&buffer : _used_buffers[index.name]) {
        PageHandle page{buffer.second, buffer.first, nullptr};
        if (_page_manager.not_flushed(page)) {
            _page_manager.flush_page(page);
        }
    }
    _used_buffers.erase(index.name);
    _page_manager.close_file(index.file_handle);
}

bool IndexManager::is_index_open(const std::string &name) const noexcept {
    return _used_buffers.count(name) != 0;
}

}
