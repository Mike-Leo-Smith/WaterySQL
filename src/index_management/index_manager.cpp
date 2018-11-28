//
// Created by Mike Smith on 2018/11/24.
//

#include <stack>

#include "index_manager.h"
#include "index_entry_offset.h"
#include "../errors/index_manager_error.h"
#include "../errors/page_manager_error.h"
#include "../utility/io_helpers/error_printer.h"
#include "../utility/memory_mapping/memory_mapper.h"
#include "../data_storage/varchar.h"

namespace watery {

void IndexManager::create_index(const std::string &name, DataDescriptor key_descriptor) {
    auto kl = key_descriptor.size;
    auto pl = static_cast<uint32_t>(sizeof(IndexEntryOffset));
    auto cpn = (PAGE_SIZE - sizeof(IndexNodeHeader) - 8) / (kl + pl) / 2 * 2;  // rounded to even
    
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
    
    MemoryMapper::map_memory<IndexHeader>(page.data) = {key_descriptor, 1, kl, pl, static_cast<uint32_t>(cpn), -1};
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
    _page_manager.mark_page_dirty(page_handle);
    _used_buffers[index.name].emplace(page_handle.buffer_handle, page_handle.buffer_offset);
    index.header.page_count++;
    return page_handle;
}

IndexNode &IndexManager::_map_index_node_page(const PageHandle &page_handle) const {
    return MemoryMapper::map_memory<IndexNode>(page_handle.data);
}

std::unique_ptr<Data> IndexManager::_get_index_entry_key(const Index &index, const IndexNode &node, ChildOffset i) {
    return Data::decode(index.header.key_descriptor, &node.fields[_get_child_key_position(index, i)]);
}

RecordOffset IndexManager::_get_index_entry_record_offset(const Index &index, const IndexNode &node, ChildOffset i) {
    return MemoryMapper::map_memory<RecordOffset>(&node.fields[_get_child_pointer_position(index, i)]);
}

uint32_t IndexManager::_get_child_pointer_position(const Index &index, ChildOffset i) {
    return i * (index.header.pointer_length + index.header.key_length);
}

uint32_t IndexManager::_get_child_key_position(const Index &index, ChildOffset i) {
    return _get_child_pointer_position(index, i) + index.header.pointer_length;
}

PageOffset IndexManager::_get_index_entry_page_offset(const Index &index, IndexNode &node, ChildOffset i) {
    return MemoryMapper::map_memory<IndexEntryOffset>(&node.fields[_get_child_pointer_position(index, i)]).page_offset;
}

void IndexManager::_write_index_entry(
    const Index &index, IndexNode &node, ChildOffset i,
    const std::unique_ptr<Data> &data, RecordOffset &record_offset) {
    _write_index_entry_key(index, node, i, data);
    MemoryMapper::map_memory<RecordOffset>(&node.fields[_get_child_pointer_position(index, i)]) = record_offset;
}

void IndexManager::_move_trailing_index_entries(
    const Index &index, IndexNode &src_node, ChildOffset src_i, IndexNode &dest_node, ChildOffset dest_i) {
    auto *src = &src_node.fields[0] + _get_child_pointer_position(index, src_i);
    auto *src_end = &src_node.fields[0] + _get_child_key_position(index, src_node.header.key_count);
    auto *dest = &dest_node.fields[0] + _get_child_pointer_position(index, dest_i);
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
        _write_index_entry(index, root_node, 0, data, record_offset);
        _write_index_node_link(index, root_node, {-1, -1});
    } else {
        static thread_local std::vector<IndexEntryOffset> path;  // stack nodes down the path for split operations.
        path.clear();
        for (auto page_offset = index.header.root_offset;;) {
            auto page_handle = _get_node_page(index, page_offset);
            auto &node = _map_index_node_page(page_handle);
            _page_manager.mark_page_accessed(page_handle);
            auto child_offset = _search_entry_in_node(index, node, data);
            path.emplace_back(page_offset, child_offset);
            if (node.header.is_leaf) { break; }  // search terminates at leaf.
            page_offset = _get_index_entry_page_offset(index, node, child_offset);
        }
        
        auto leaf_offset = path.back();
        path.pop_back();
        
        auto leaf_page = _get_node_page(index, leaf_offset.page_offset);
        auto &leaf = _map_index_node_page(leaf_page);
        _page_manager.mark_page_dirty(leaf_page);
        
        if (leaf.header.key_count < index.header.key_count_per_node) {  // can be insert directly.
            _move_trailing_index_entries(index, leaf, leaf_offset.child_offset, leaf, leaf_offset.child_offset + 1);
            _write_index_entry(index, leaf, leaf_offset.child_offset, data, record_offset);
            leaf.header.key_count++;
            return;
        }
        
        // split and insert for the leaf.
        auto split_leaf_page = _allocate_node_page(index);  // the allocated page will be marked dirty.
        auto &split_leaf = _map_index_node_page(split_leaf_page);
        split_leaf.header.is_leaf = true;
        // decide where to split.
        if (leaf_offset.child_offset <= index.header.key_count_per_node / 2) {  // insert into the former part.
            auto split_pos = index.header.key_count_per_node / 2;
            // move the trailing part to the split page.
            _move_trailing_index_entries(index, leaf, split_pos, split_leaf, 0);
            // update entry count.
            split_leaf.header.key_count = leaf.header.key_count - split_pos;
            leaf.header.key_count = split_pos;
            // make space for the new entry.
            _move_trailing_index_entries(index, leaf, leaf_offset.child_offset, leaf, leaf_offset.child_offset + 1);
            _write_index_entry(index, leaf, leaf_offset.child_offset, data, record_offset);
            // update entry count.
            leaf.header.key_count++;
        } else {  // into the latter part.
            auto split_pos = index.header.key_count_per_node / 2 + 1;
            auto ins_pos = leaf_offset.child_offset;
            auto split_entry_count = leaf.header.key_count - split_pos + 1;
            _move_trailing_index_entries(index, leaf, ins_pos, split_leaf, ins_pos - split_pos + 1);
            leaf.header.key_count = static_cast<uint32_t>(ins_pos);
            _move_trailing_index_entries(index, leaf, split_pos, split_leaf, 0);
            _write_index_entry(index, split_leaf, ins_pos - split_pos, data, record_offset);
            leaf.header.key_count = split_pos;
            split_leaf.header.key_count = split_entry_count;
        }
        
        // link with the split leaf.
        auto link = _get_index_node_link(index, split_leaf);
        IndexNodeLink leaf_link{link.prev, split_leaf_page.buffer_offset.page_offset};
        IndexNodeLink split_link{leaf_offset.page_offset, link.next};
        _write_index_node_link(index, leaf, leaf_link);
        _write_index_node_link(index, split_leaf, split_link);
        
        // pop up squeezed entry.
        auto k = _get_index_entry_key(index, leaf, leaf.header.key_count - 1);
        auto split_page_offset = split_leaf_page.buffer_offset.page_offset;
        
        // insert popped up entry into upper nodes.
        while (!path.empty()) {
            
            // fetch the associated node.
            auto e = path.back();
            path.pop_back();
            auto page_handle = _get_node_page(index, e.page_offset);
            auto &node = _map_index_node_page(page_handle);
            _page_manager.mark_page_dirty(page_handle);
            
            // since the child node was split, the pointer changes.
            auto former_offset = _get_index_entry_page_offset(index, node, e.child_offset);  // back-up
            _write_index_entry_page_offset(index, node, e.child_offset, split_page_offset);
            
            if (node.header.key_count < index.header.key_count_per_node) {  // insert directly.
                _move_trailing_index_entries(index, node, e.child_offset, node, e.child_offset + 1);
                _write_index_entry_key(index, node, e.child_offset, k);
                node.header.key_count++;
                // update child pointers.
                _write_index_entry_page_offset(index, node, e.child_offset, former_offset);
                return;
            }
            
            auto split_page = _allocate_node_page(index);
            auto &split_node = _map_index_node_page(split_page);
            split_node.header.is_leaf = false;
            
            // split and insert.
            if (e.child_offset <= index.header.key_count_per_node / 2) {  // insert into the former part.
                auto split_pos = index.header.key_count_per_node / 2;
                // move entries to split node to make space for insertion.
                _move_trailing_index_entries(index, node, split_pos, split_node, 0);
                // update entry count.
                split_node.header.key_count = node.header.key_count - split_pos;
                node.header.key_count = split_pos;
                // make space for the new entry.
                _move_trailing_index_entries(index, node, e.child_offset, node, e.child_offset + 1);
                _write_index_entry_key(index, node, e.child_offset, k);
                _write_index_entry_page_offset(index, node, e.child_offset, former_offset);
                // update entry count.
                node.header.key_count++;
            } else {
                auto split_pos = index.header.key_count_per_node / 2 + 1;
                auto split_entry_count = node.header.key_count - split_pos + 1;
                _move_trailing_index_entries(index, node, e.child_offset, split_node, e.child_offset - split_pos + 1);
                node.header.key_count = static_cast<uint32_t>(e.child_offset);
                _move_trailing_index_entries(index, node, split_pos, split_node, 0);
                _write_index_entry_key(index, split_node, e.child_offset - split_pos, k);
                _write_index_entry_page_offset(index, split_node, e.child_offset - split_pos, former_offset);
                node.header.key_count = split_pos;
                split_node.header.key_count = split_entry_count;
            }
            
            // squeeze
            k = _get_index_entry_key(index, node, node.header.key_count - 1);
            split_page_offset = split_page.buffer_offset.page_offset;
            node.header.key_count--;
        }
        
        // reach root, new root should be allocate.
        auto root_page = _allocate_node_page(index);
        auto &root = _map_index_node_page(root_page);
        root.header.key_count = 1;
        _write_index_entry_key(index, root, 0, k);
        _write_index_entry_page_offset(index, root, 0, index.header.root_offset);
        _write_index_entry_page_offset(index, root, 1, split_page_offset);
        index.header.root_offset = root_page.buffer_offset.page_offset;
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
    if (record_offset == rid &&
        entry_offset.child_offset < node.header.key_count &&
        *_get_index_entry_key(index, node, entry_offset.child_offset) == *data) {
        _move_trailing_index_entries(index, node, entry_offset.child_offset + 1, node, entry_offset.child_offset);
        node.header.key_count--;
        _page_manager.mark_page_dirty(page_handle);
    } else {
        throw IndexManagerError{"Failed to delete index entry that may not exist."};
    }
}

IndexEntryOffset IndexManager::_search_entry_in(Index &index, PageOffset p, const std::unique_ptr<Data> &data) {
    while (true) {
        auto page_handle = _get_node_page(index, p);
        auto &node = _map_index_node_page(page_handle);
        auto child_offset = _search_entry_in_node(index, node, data);
        if (node.header.is_leaf) {
            return {p, child_offset};
        }
        p = _get_index_entry_page_offset(index, node, child_offset);
    }
}

ChildOffset IndexManager::_search_entry_in_node(Index &index, const IndexNode &node, const std::unique_ptr<Data> &k) {
    auto left = 0;
    auto right = node.header.key_count;
    while (left < right) {
        auto mid = (left + right) / 2;
        if (*_get_index_entry_key(index, node, mid) < *k) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return right;
}

void IndexManager::_write_index_entry_page_offset(const Index &idx, IndexNode &n, ChildOffset i, PageOffset p) {
    MemoryMapper::map_memory<RecordOffset>(&n.fields[_get_child_pointer_position(idx, i)]).page_offset = p;
}

void IndexManager::_write_index_node_link(const Index &idx, IndexNode &n, IndexNodeLink l) {
    MemoryMapper::map_memory<IndexNodeLink>(&n.fields[_get_child_pointer_position(idx, n.header.key_count)]) = l;
}

IndexNodeLink IndexManager::_get_index_node_link(const Index &idx, const IndexNode &n) {
    return MemoryMapper::map_memory<IndexNodeLink>(&n.fields[_get_child_pointer_position(idx, n.header.key_count)]);
}

void IndexManager::_write_index_entry_key(
    const Index &idx, IndexNode &n, ChildOffset i, const std::unique_ptr<Data> &k) {
    k->encode(&n.fields[_get_child_key_position(idx, i)]);
}

IndexEntryOffset IndexManager::search_index_entry(Index &index, const std::unique_ptr<Data> &data) {
    if (index.header.root_offset == -1) {  // searching in empty tree.
        throw IndexManagerError{"Failed to search index entry in an empty index tree."};
    }
    return _search_entry_in(index, index.header.root_offset, data);
}

}
