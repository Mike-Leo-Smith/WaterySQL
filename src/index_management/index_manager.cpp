//
// Created by Mike Smith on 2018/11/24.
//

#include <stack>

#include "index_manager.h"
#include "index_node_pointer_offset.h"
#include "../errors/index_manager_error.h"
#include "../errors/page_manager_error.h"
#include "../utility/io_helpers/error_printer.h"
#include "../utility/memory_mapping/memory_mapper.h"

namespace watery {

void IndexManager::create_index(const std::string &name, DataDescriptor key_descriptor) {
    auto kl = key_descriptor.size;
    auto pl = static_cast<uint32_t>(sizeof(IndexEntryOffset));
    auto cpn = std::min(MAX_CHILD_COUNT_PER_INDEX_NODE,
                        static_cast<uint32_t>((PAGE_SIZE - sizeof(IndexNodeHeader)) / (kl + pl)));
    
    if (cpn == 0) {
        throw IndexManagerError{
            std::string{"Failed to create index because the keys are too long ("}
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
    
    MemoryMapper::map_memory<IndexHeader>(page.data) = {key_descriptor, 1, kl, pl, cpn, -1};
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
    _used_buffers[name].emplace(header_page.buffer_handle, header_page.buffer_offset);
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

PageHandle IndexManager::_get_node_page(const Index &index, PageOffset node_offset) {
    auto page_handle = _page_manager.get_page(index.file_handle, node_offset);
    _page_manager.mark_page_accessed(page_handle);
    _used_buffers[index.name].emplace(page_handle.buffer_handle, page_handle.buffer_offset);
    return page_handle;
}

PageHandle IndexManager::_allocate_node_page(Index &index) {
    auto page_handle = _page_manager.allocate_page(index.file_handle, index.header.page_count);
    _used_buffers[index.name].emplace(page_handle.buffer_handle, page_handle.buffer_offset);
    _page_manager.mark_page_dirty(page_handle);
    index.header.page_count++;
    return page_handle;
}

IndexNode &IndexManager::_map_index_node_page(const PageHandle &page_handle) const {
    return MemoryMapper::map_memory<IndexNode>(page_handle.data);
}

std::unique_ptr<Data> IndexManager::_get_index_entry_key(const Index &index, const IndexNode &node, ChildOffset i) {
    return Data::decode(index.header.key_descriptor, &node.fields[_get_child_key_offset(index, i)]);
}

RecordOffset IndexManager::_get_index_entry_record_offset(const Index &index, const IndexNode &node, ChildOffset i) {
    return MemoryMapper::map_memory<RecordOffset>(&node.fields[_get_child_pointer_offset(index, i)]);
}

inline uint32_t IndexManager::_get_child_pointer_offset(const Index &index, ChildOffset i) {
    return i * (index.header.pointer_length + index.header.key_length);
}

inline uint32_t IndexManager::_get_child_key_offset(const Index &index, ChildOffset i) {
    return _get_child_pointer_offset(index, i) + index.header.pointer_length;
}

PageOffset IndexManager::_get_child_page_offset(const Index &index, IndexNode &node, ChildOffset i) {
    return MemoryMapper::map_memory<IndexEntryOffset>(&node.fields[_get_child_pointer_offset(index, i)]).page_offset;
}

void IndexManager::_write_index_entry(
    const Index &index, IndexNode &node, ChildOffset i,
    const std::unique_ptr<Data> &data, RecordOffset &record_offset) {
    data->encode(node.fields + _get_child_key_offset(index, i));
    MemoryMapper::map_memory<RecordOffset>(node.fields + _get_child_pointer_offset(index, i)) = record_offset;
}

void IndexManager::_move_trailing_index_entries(
    const Index &index, IndexNode &src_node, ChildOffset src_i, IndexNode &dest_node, ChildOffset dest_i) {
    auto *src = &src_node.fields[0] + _get_child_pointer_offset(index, src_i);
    auto *src_end = &src_node.fields[0] + _get_child_key_offset(index, src_node.header.key_count);
    auto *dest = &dest_node.fields[0] + _get_child_pointer_offset(index, dest_i);
    std::memmove(dest, src, src_end - src);
}

void IndexManager::insert_index_entry(Index &index, const std::unique_ptr<Data> &data, RecordOffset record_offset) {
    if (index.header.root_offset == -1) {  // empty, create new root
        auto root_page_handle = _allocate_node_page(index);
        _page_manager.mark_page_accessed(root_page_handle);
        auto &root_node = _map_index_node_page(root_page_handle);
        index.header.root_offset = 1;
        root_node.header.key_count = 1;
        root_node.header.is_leaf = true;
        root_node.header.parent_page_offset = -1;
        _write_index_entry(index, root_node, 0, data, record_offset);
    } else {
        std::stack<PageOffset> path;  // stack nodes down the path for further split operations.
        
        for (auto page_offset = index.header.root_offset;;) {
            path.emplace(page_offset);
            auto page_handle = _get_node_page(index, page_offset);
            _page_manager.mark_page_accessed(page_handle);
            auto &node = _map_index_node_page(page_handle);
            if (node.header.is_leaf) { break; }
            auto child_offset = _search_entry_in_node(index, node, data);
            page_offset = _get_child_page_offset(index, node, child_offset);
        }
        
        auto k = data->replica();
        auto e = record_offset;
        while (!path.empty()) {
            auto page_offset = path.top();
            path.pop();
            auto page_handle = _get_node_page(index, page_offset);
            _page_manager.mark_page_dirty(page_handle);
            auto &node = _map_index_node_page(page_handle);
            auto child_offset = _search_entry_in_node(index, node, k);  // find offset for insertion insert.
            
            if (node.header.key_count < index.header.key_count_per_node) {  // can be insert directly.
                _move_trailing_index_entries(index, node, child_offset, node, child_offset + 1);
                _write_index_entry(index, node, child_offset, k, e);
                node.header.key_count++;
                return;
            }
            
            // split and insert.
            auto split_page = _allocate_node_page(index);
            auto split_node = _map_index_node_page(split_page);
            split_node.header.is_leaf = node.header.is_leaf;
            if (child_offset <= index.header.key_count_per_node / 2) {
                auto split_pos = index.header.key_count_per_node / 2;
                
                // move trailing entries to the new page.
                _move_trailing_index_entries(index, node, split_pos, split_node, 0);
                split_node.header.key_count = node.header.key_count - split_pos;
                node.header.key_count = split_pos;
                
                // link split node.
//                _write_index_entry_page_offset(index, node, split_pos, split_page.buffer_offset.page_offset);
                
                // insert new entry.
                _move_trailing_index_entries(index, node, child_offset, node, child_offset + 1);
                _write_index_entry(index, node, child_offset, k, e);
                node.header.key_count++;
                
                // update k and e for upper level.
                k = _get_index_entry_key(index, node, split_pos);
                e = {page_offset, -1};
            } else {
                auto split_pos = index.header.key_count_per_node / 2 + 1;
                
                // move entries.
                _move_trailing_index_entries(index, node, split_pos, split_node, 0);
                split_node.header.key_count = node.header.key_count - split_pos;
                node.header.key_count = split_pos;
                
                // link.
//                _write_index_entry_page_offset(index, node, split_pos, split_page.buffer_offset.page_offset);
            
            
            }
            
        }
    }
}

void IndexManager::delete_index_entry(Index &index, const std::unique_ptr<Data> &data, RecordOffset record_offset) {
    if (index.header.root_offset == -1) {
        throw IndexManagerError{"Failed to delete index entry in an empty index tree."};
    }
    auto entry_offset = _search_entry_in(index, index.header.root_offset, data);
    auto page_handle = _get_node_page(index, entry_offset.page_offset);
    auto &node = _map_index_node_page(page_handle);
    auto rid = _get_index_entry_record_offset(index, node, entry_offset.child_offset);
    if (entry_offset.child_offset < node.header.key_count && record_offset == rid) {
        _move_trailing_index_entries(index, node, entry_offset.child_offset + 1, node, entry_offset.child_offset);
        node.header.key_count--;
        _page_manager.mark_page_dirty(page_handle);
    } else {
        throw IndexManagerError{"Failed to delete index entry that may not exist."};
    }
}

IndexEntryOffset IndexManager::_search_entry_in(Index &index, PageOffset p, const std::unique_ptr<Data> &data) {
    auto page_handle = _get_node_page(index, p);
    auto &node = _map_index_node_page(page_handle);
    auto child_offset = _search_entry_in_node(index, node, data);
    return node.header.is_leaf ?
           IndexEntryOffset{p, child_offset} :
           _search_entry_in(index, child_offset, data);
}

IndexEntryOffset IndexManager::search_index_entry(Index &index, const std::unique_ptr<Data> &data) {
    if (index.header.root_offset == -1) {  // searching in empty tree.
        throw IndexManagerError{"Failed to search index entry in an empty index tree."};
    }
    return _search_entry_in(index, index.header.root_offset, data);
}

ChildOffset IndexManager::_search_entry_in_node(Index &index, const IndexNode &node, const std::unique_ptr<Data> &k) {
    for (auto i = 0; i < node.header.key_count; i++) {
        auto key = _get_index_entry_key(index, node, i);
        if (!(*key < *k)) {
            return i;
        }
    }
    return node.header.key_count;
}

void IndexManager::_write_index_entry_page_offset(const Index &idx, IndexNode &n, ChildOffset i, PageOffset p) {
    MemoryMapper::map_memory<RecordOffset>(&n.fields[_get_child_pointer_offset(idx, i)]).page_offset = p;
}

void IndexManager::_write_index_node_link(const Index &idx, IndexNode &n, IndexNodeLink l) {
    MemoryMapper::map_memory<IndexNodeLink>(&n.fields[_get_child_pointer_offset(idx, n.header.key_count)]) = l;
}

IndexNodeLink IndexManager::_get_index_node_link(const Index &idx, const IndexNode &n) {
    return MemoryMapper::map_memory<IndexNodeLink>(&n.fields[_get_child_pointer_offset(idx, n.header.key_count)]);
}

}
