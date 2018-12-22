//
// Created by Mike Smith on 2018/11/24.
//

#include <cstring>

#include "index_manager.h"
#include "index_entry_offset.h"
#include "../errors/index_manager_error.h"
#include "../errors/page_manager_error.h"
#include "../utility/io/error_printer.h"
#include "../utility/memory/memory_mapper.h"
#include "../utility/mathematics/sgn.h"

namespace watery {

void IndexManager::create_index(const std::string &name, DataDescriptor data_desc, bool unique) {
    
    auto dl = data_desc.length();
    auto kl = static_cast<uint32_t>(unique ? dl : dl + sizeof(RecordOffset));
    auto pl = static_cast<uint32_t>(sizeof(RecordOffset));
    auto cpn = (PAGE_SIZE - sizeof(IndexNodeHeader) - 8 /* for alignment */) / (kl + pl) / 2 * 2;  // rounded to even
    
    if (cpn == 0) {
        throw IndexManagerError{
            std::string{"Failed to create index because the keys are too long ("}
                .append(std::to_string(kl)).append(" bytes).")};
    }
    
    auto file_name = name + INDEX_FILE_EXTENSION;
    
    if (_page_manager.file_exists(file_name)) {
        throw IndexManagerError{
            std::string{"Failed to create file for index \""}.append(name).append("\" which already exists.")};
    }
    
    FileHandle file_handle;
    try {
        _page_manager.create_file(file_name);
        file_handle = _page_manager.open_file(file_name);
    } catch (const PageManagerError &e) {
        print_error(std::cerr, e);
        throw IndexManagerError{std::string{"Failed to create file for index \""}.append(name).append("\".")};
    }
    
    auto page = _page_manager.allocate_page(file_handle, 0);
    
    MemoryMapper::map_memory<IndexHeader>(page.data) = {data_desc, unique, kl, dl, 1, static_cast<uint32_t>(cpn), -1};
    _page_manager.mark_page_dirty(page);
    _page_manager.flush_page(page);
    _page_manager.close_file(file_handle);
}

void IndexManager::delete_index(const std::string &name) {
    if (is_index_open(name)) {
        close_index(name);
    }
    try {
        auto file_name = name + INDEX_FILE_EXTENSION;
        _page_manager.delete_file(file_name);
    } catch (const PageManagerError &e) {
        print_error(std::cerr, e);
        throw IndexManagerError{std::string{"Failed to delete the file for index \""}.append(name).append("\".")};
    }
}

Index &IndexManager::open_index(const std::string &name) {
    if (!is_index_open(name)) {
        FileHandle file_handle;
        try {
            auto file_name = name + INDEX_FILE_EXTENSION;
            file_handle = _page_manager.open_file(file_name);
        } catch (const PageManagerError &e) {
            print_error(std::cerr, e);
            throw IndexManagerError(std::string{"Failed to open file for table \""}.append(name).append("\"."));
        }
    
        // load table header
        auto header_page = _page_manager.get_page(file_handle, 0);
        _page_manager.mark_page_accessed(header_page);
        _used_buffers[file_handle].emplace(header_page.buffer_handle, header_page.buffer_offset);
        auto index_header = MemoryMapper::map_memory<IndexHeader>(header_page.data);
        _open_indices.emplace(name, Index{name, file_handle, index_header});
    }
    return _open_indices[name];
}

void IndexManager::close_index(const std::string &name) noexcept {
    if (!is_index_open(name)) {
        return;
    }
    _close_index(_open_indices[name]);
    _open_indices.erase(name);
}

void IndexManager::_close_index(const Index &index) noexcept {
    // update table header
    auto header_page = _page_manager.get_page(index.file_handle, 0);
    MemoryMapper::map_memory<IndexHeader>(header_page.data) = index.header;
    _page_manager.mark_page_dirty(header_page);
    _page_manager.flush_page(header_page);
    
    for (auto &&buffer : _used_buffers[index.file_handle]) {
        PageHandle page{buffer.second, buffer.first, nullptr};
        if (_page_manager.not_flushed(page)) {
            _page_manager.flush_page(page);
        }
    }
    _used_buffers[index.file_handle].clear();
    _page_manager.close_file(index.file_handle);
}

bool IndexManager::is_index_open(const std::string &name) const noexcept {
    return _open_indices.count(name) != 0;
}

PageHandle IndexManager::_get_node_page(const Index &index, PageOffset node_offset) noexcept {
    auto page_handle = _page_manager.get_page(index.file_handle, node_offset);
    _page_manager.mark_page_accessed(page_handle);
    _used_buffers[index.file_handle].emplace(page_handle.buffer_handle, page_handle.buffer_offset);
    return page_handle;
}

PageHandle IndexManager::_allocate_node_page(Index &index) noexcept {
    auto page_handle = _page_manager.allocate_page(index.file_handle, index.header.page_count);
    _page_manager.mark_page_dirty(page_handle);
    _used_buffers[index.file_handle].emplace(page_handle.buffer_handle, page_handle.buffer_offset);
    index.header.page_count++;
    return page_handle;
}

IndexNode &IndexManager::_map_index_node_page(const PageHandle &page_handle) noexcept {
    return MemoryMapper::map_memory<IndexNode>(page_handle.data);
}

const Byte *IndexManager::_get_index_entry_key(
    const Index &index, const IndexNode &node, ChildOffset i) noexcept {
    return &node.fields[_get_child_key_position(index, i)];
}

RecordOffset IndexManager::_get_index_entry_record_offset(
    const Index &index, const IndexNode &node, ChildOffset i) noexcept {
    return MemoryMapper::map_memory<RecordOffset>(&node.fields[_get_child_pointer_position(index, i)]);
}

uint32_t IndexManager::_get_child_pointer_position(const Index &index, ChildOffset i) noexcept {
    return i * (sizeof(RecordOffset) + index.header.key_length);
}

uint32_t IndexManager::_get_child_key_position(const Index &index, ChildOffset i) noexcept {
    return _get_child_pointer_position(index, i) + sizeof(RecordOffset);
}

PageOffset IndexManager::_get_index_entry_page_offset(const Index &index, IndexNode &node, ChildOffset i) noexcept {
    return MemoryMapper::map_memory<IndexEntryOffset>(&node.fields[_get_child_pointer_position(index, i)]).page_offset;
}

void IndexManager::_move_trailing_index_entries(
    const Index &index, IndexNode &src_node, ChildOffset src_i, IndexNode &dest_node, ChildOffset dest_i) noexcept {
    auto *src = &src_node.fields[0] + _get_child_pointer_position(index, src_i);
    auto *src_end = &src_node.fields[0] + _get_child_key_position(index, src_node.header.key_count);
    auto *dest = &dest_node.fields[0] + _get_child_pointer_position(index, dest_i);
    std::memmove(dest, src, src_end - src);
}

void IndexManager::insert_index_entry(Index &index, const Byte *data, RecordOffset rid) {
    thread_local static std::vector<Byte> key_compact;
    _make_key_compact(index, key_compact, data, rid);
    if (index.header.root_offset == -1) {  // empty, create new root
        auto root_page_handle = _allocate_node_page(index);
        _page_manager.mark_page_accessed(root_page_handle);
        auto &root_node = _map_index_node_page(root_page_handle);
        index.header.root_offset = 1;
        root_node.header.key_count = 1;
        root_node.header.is_leaf = true;
        IndexNode &node = root_node;
        _write_index_entry_key(index, node, 0, key_compact.data(), rid);
        _write_index_entry_record_offset(index, node, 0, rid);
        _write_index_node_link(index, root_node, {-1, -1});
    } else {
        static thread_local std::vector<IndexEntryOffset> path;  // stack nodes down the path for split operations.
        path.clear();
        
        for (auto page_offset = index.header.root_offset;;) {
            auto page_handle = _get_node_page(index, page_offset);
            auto &node = _map_index_node_page(page_handle);
            _page_manager.mark_page_accessed(page_handle);
            auto child_offset = _search_entry_in_node(index, node, key_compact.data());
            path.emplace_back(page_offset, child_offset);
            if (node.header.is_leaf) { break; }  // search terminates at leaf.
            page_offset = _get_index_entry_page_offset(index, node, child_offset);
        }
        
        // Note:
        // Since the height of B+ trees are effectively low, we do not
        // consider the possibility that the page k is bound to will be
        // swapped during the insertion for efficiency, but still this
        // can be thought as a potential vulnerability.
        const auto *k = key_compact.data();
        auto split_page_offset = 0;
        
        // insert popped up entry into upper nodes.
        while (!path.empty()) {
            
            // fetch the associated node.
            auto e = path.back();
            path.pop_back();
            auto page_handle = _get_node_page(index, e.page_offset);
            auto &node = _map_index_node_page(page_handle);
            _page_manager.mark_page_dirty(page_handle);
            
            // since the child node was split, the pointer changes.
            auto page_offset = node.header.is_leaf ? rid.page_offset :
                               _get_index_entry_page_offset(index, node, e.child_offset);  // back-up
            auto slot_offset = node.header.is_leaf ? rid.slot_offset : -1;
            
            if (!node.header.is_leaf) {
                _write_index_entry_page_offset(index, node, e.child_offset, split_page_offset);
            }
            
            if (node.header.key_count < index.header.key_count_per_node) {  // insert directly.
                _move_trailing_index_entries(index, node, e.child_offset, node, e.child_offset + 1);
                _write_index_entry_key(index, node, e.child_offset, k, rid);
                _write_index_entry_record_offset(index, node, e.child_offset, {page_offset, slot_offset});
                node.header.key_count++;
                return;
            }
            
            // allocate page for split node.
            auto split_page = _allocate_node_page(index);
            auto &split_node = _map_index_node_page(split_page);
            split_node.header.is_leaf = node.header.is_leaf;
            
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
                _write_index_entry_key(index, node, e.child_offset, k, rid);
                _write_index_entry_record_offset(index, node, e.child_offset, {page_offset, slot_offset});
                // update entry count.
                node.header.key_count++;
            } else {
                auto split_pos = index.header.key_count_per_node / 2 + 1;
                auto split_entry_count = node.header.key_count - split_pos + 1;
                auto split_child_offset = e.child_offset - split_pos;
                _move_trailing_index_entries(index, node, e.child_offset, split_node, split_child_offset + 1);
                node.header.key_count = static_cast<uint32_t>(e.child_offset);
                _move_trailing_index_entries(index, node, split_pos, split_node, 0);
                _write_index_entry_key(index, split_node, split_child_offset, k, rid);
                _write_index_entry_record_offset(index, split_node, split_child_offset, {page_offset, slot_offset});
                node.header.key_count = split_pos;
                split_node.header.key_count = split_entry_count;
            }
            
            // squeeze
            k = _get_index_entry_key(index, node, node.header.key_count - 1);
            split_page_offset = split_page.buffer_offset.page_offset;
            if (node.header.is_leaf) {  // link nodes when it is a leaf to form linked-lists.
                auto link = _get_index_node_link(index, split_node);
                IndexNodeLink leaf_link{link.prev, split_page.buffer_offset.page_offset};
                IndexNodeLink split_link{e.page_offset, link.next};
                _write_index_node_link(index, node, leaf_link);
                _write_index_node_link(index, split_node, split_link);
            } else {  // when it is an internal node, just squeeze it.
                node.header.key_count--;
            }
        }
        
        // reach root, new root should be allocate.
        auto root_page = _allocate_node_page(index);
        auto &root = _map_index_node_page(root_page);
        root.header.key_count = 1;
        _write_index_entry_key(index, root, 0, k, rid);
        _write_index_entry_page_offset(index, root, 0, index.header.root_offset);
        _write_index_entry_page_offset(index, root, 1, split_page_offset);
        index.header.root_offset = root_page.buffer_offset.page_offset;
    }
}

void IndexManager::delete_index_entry(Index &index, const Byte *data, RecordOffset rid) {
    thread_local static std::vector<Byte> key_compact;
    if (index.header.root_offset == -1) {
        throw IndexManagerError{"Failed to delete index entry in an empty index tree."};
    }
    _make_key_compact(index, key_compact, data, rid);
    auto entry_offset = _search_entry_in(index, index.header.root_offset, key_compact.data());
    auto page_handle = _get_node_page(index, entry_offset.page_offset);
    auto &node = _map_index_node_page(page_handle);
    if (entry_offset.child_offset < node.header.key_count &&
        rid == _get_index_entry_record_offset(index, node, entry_offset.child_offset)) {
        _move_trailing_index_entries(index, node, entry_offset.child_offset + 1, node, entry_offset.child_offset);
        node.header.key_count--;
        _page_manager.mark_page_dirty(page_handle);
    } else {
        throw IndexManagerError{"Failed to delete index entry that may not exist."};
    }
}

IndexEntryOffset IndexManager::_search_entry_in(Index &index, PageOffset p, const Byte *data) noexcept {
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

ChildOffset IndexManager::_search_entry_in_node(Index &index, const IndexNode &node, const Byte *k) noexcept {
    auto left = 0;
    auto right = node.header.key_count;
    while (left < right) {
        auto mid = left + (right - left) / 2;
        if (index.comparator.less(_get_index_entry_key(index, node, mid), k)) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return right;
}

void IndexManager::_write_index_entry_page_offset(
    const Index &idx, IndexNode &n, ChildOffset i, PageOffset p) noexcept {
    MemoryMapper::map_memory<PageOffset>(&n.fields[_get_child_pointer_position(idx, i)]) = p;
}

void IndexManager::_write_index_node_link(const Index &idx, IndexNode &n, IndexNodeLink l) noexcept {
    MemoryMapper::map_memory<IndexNodeLink>(&n.fields[_get_child_pointer_position(idx, n.header.key_count)]) = l;
}

IndexNodeLink IndexManager::_get_index_node_link(const Index &idx, const IndexNode &n) noexcept {
    return MemoryMapper::map_memory<IndexNodeLink>(&n.fields[_get_child_pointer_position(idx, n.header.key_count)]);
}

void IndexManager::_write_index_entry_key(
    const Index &idx, IndexNode &n, ChildOffset i, const Byte *k, RecordOffset rid) noexcept {
    auto p = &n.fields[_get_child_key_position(idx, i)];
    std::memmove(p, k, idx.header.key_length);
}

IndexEntryOffset IndexManager::search_index_entry(Index &index, const Byte *data) {
    thread_local static std::vector<Byte> key_compact;
    if (index.header.root_offset == -1) {  // searching in empty tree.
        throw IndexManagerError{"Failed to search index entry in an empty index tree."};
    }
    _make_key_compact(index, key_compact, data, {-1, -1});
    return _search_entry_in(index, index.header.root_offset, key_compact.data());
}

void IndexManager::_make_key_compact(
    const Index &index, std::vector<Byte> &key_compact, const Byte *data, RecordOffset rid) const {
    key_compact.resize(index.header.key_length);
    if (index.header.unique) {
        memmove(key_compact.data(), data, index.header.key_length);
    } else {  // composed key
        memmove(key_compact.data(), &rid, sizeof(RecordOffset));
        memmove(key_compact.data() + sizeof(RecordOffset), data, index.header.data_length);
    }
}

void IndexManager::_write_index_entry_record_offset(
    const Index &idx, IndexNode &n, ChildOffset i, RecordOffset r) noexcept {
    MemoryMapper::map_memory<RecordOffset>(&n.fields[_get_child_pointer_position(idx, i)]) = r;
}

IndexManager::~IndexManager() {
    close_all_indices();
}

void IndexManager::close_all_indices() noexcept {
    for (auto &&entry : _open_indices) {
        _close_index(entry.second);
    }
    _open_indices.clear();
}

}
