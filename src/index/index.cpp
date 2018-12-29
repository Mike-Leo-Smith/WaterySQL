//
// Created by Mike Smith on 2018-12-27.
//

#include <cstring>

#include "index.h"
#include "../page/page_manager.h"
#include "../error/empty_index_tree.h"
#include "../error/index_entry_offset_out_of_range.h"
#include "../error/index_entry_not_found.h"
#include "../error/unique_search_in_non_unique_index.h"
#include "../error/conflict_index_entry_insertion.h"
#include "../utility/io/printer.h"

namespace watery {

watery::Index::~Index() {
    auto cache_handle = PageManager::instance().load_page({(_file_handle), 0});
    auto cache = PageManager::instance().access_cache_for_writing(cache_handle);
    MemoryMapper::map_memory<IndexHeader>(cache) = _header;
    PageManager::instance().close_file(_file_handle);
}

Index::Index(std::string name, FileHandle fh, IndexHeader h)
    : _name{std::move(name)}, _file_handle{fh}, _header{h}, _comparator{h.key_descriptor, h.unique} {}

IndexNode &Index::_allocate_index_node() {
    auto page_offset = static_cast<PageOffset>(_header.page_count++);
    auto page_cache_handle = PageManager::instance().allocate_page({_file_handle, page_offset});
    auto page_cache = PageManager::instance().access_cache_for_writing(page_cache_handle);
    return MemoryMapper::map_memory<IndexNode>(page_cache);
}

IndexEntryOffset Index::_search_entry_in(PageOffset p, const Byte *data) const {
    while (true) {
        const auto &node = _load_node_for_reading(_file_handle, p);
        auto child_offset = _search_entry_in_node(node, data);
        if (node.header.is_leaf) {
            return {p, child_offset};
        }
        p = _get_index_entry_page_offset(node, child_offset);
    }
}

ChildOffset Index::_search_entry_in_node(const IndexNode &node, const Byte *k) const noexcept {
    auto left = 0;
    auto right = node.header.key_count;
    while (left < right) {
        auto mid = left + (right - left) / 2;
        if (_comparator.less(_get_index_entry_key(node, mid), k)) {
            left = mid + 1;
        } else {
            right = mid;
        }
    }
    return right;
}

uint32_t Index::_get_child_pointer_position(ChildOffset i) const noexcept {
    return i * (sizeof(RecordOffset) + _header.key_length);
}

uint32_t Index::_get_child_key_position(ChildOffset i) const noexcept {
    return _get_child_pointer_position(i) + sizeof(RecordOffset);
}

PageOffset Index::_get_index_entry_page_offset(const IndexNode &node, ChildOffset i) const noexcept {
    return MemoryMapper::map_memory<IndexEntryOffset>(&node.fields[_get_child_pointer_position(i)]).page_offset;
}

const Byte *Index::_get_index_entry_key(const IndexNode &node, ChildOffset i) const noexcept {
    return &node.fields[_get_child_key_position(i)];
}

RecordOffset Index::_get_index_entry_record_offset(const IndexNode &node, ChildOffset i) const noexcept {
    return MemoryMapper::map_memory<RecordOffset>(&node.fields[_get_child_pointer_position(i)]);
}

void Index::_write_index_entry_page_offset(IndexNode &n, ChildOffset i, PageOffset p) noexcept {
    MemoryMapper::map_memory<PageOffset>(&n.fields[_get_child_pointer_position(i)]) = p;
}

void Index::_write_index_entry_record_offset(IndexNode &n, ChildOffset i, RecordOffset r) noexcept {
    MemoryMapper::map_memory<RecordOffset>(&n.fields[_get_child_pointer_position(i)]) = r;
}

void Index::_write_index_entry_key(IndexNode &n, ChildOffset i, const Byte *k) noexcept {
    std::memmove(&n.fields[_get_child_key_position(i)], k, _header.key_length);
}

void Index::_write_index_node_link(IndexNode &n, IndexNodeLink l) noexcept {
    MemoryMapper::map_memory<IndexNodeLink>(&n.fields[_get_child_pointer_position(n.header.key_count)]) = l;
}

IndexNodeLink Index::_get_index_node_link(const IndexNode &n) const noexcept {
    return MemoryMapper::map_memory<IndexNodeLink>(&n.fields[_get_child_pointer_position(n.header.key_count)]);
}

void Index::_move_trailing_index_entries(
    IndexNode &src_node, ChildOffset src_i, IndexNode &dest_node, ChildOffset dest_i) noexcept {
    auto *src = &src_node.fields[0] + _get_child_pointer_position(src_i);
    auto *src_end = &src_node.fields[0] + _get_child_key_position(src_node.header.key_count);
    auto *dest = &dest_node.fields[0] + _get_child_pointer_position(dest_i);
    std::memmove(dest, src, src_end - src);
}

const Byte *Index::_make_key_compact(const Byte *data, RecordOffset rid) const noexcept {
    thread_local static std::vector<Byte> key_compact;
    key_compact.resize(_header.key_length);
    if (_header.unique) {
        std::memmove(key_compact.data(), data, _header.key_length);
    } else {  // composed key
        std::memmove(key_compact.data(), &rid, sizeof(RecordOffset));
        std::memmove(key_compact.data() + sizeof(RecordOffset), data, _header.data_length);
    }
    return key_compact.data();
}

bool Index::_index_entry_key_matches(const IndexNode &node, ChildOffset offset, const Byte *data) const noexcept {
    return offset < node.header.key_count &&
           std::memcmp(data, _get_index_entry_key(node, offset), _header.key_length) == 0;
}

bool Index::_index_entry_data_matches(const IndexNode &node, ChildOffset offset, const Byte *data) const noexcept {
    auto d = _header.key_length - _header.data_length;
    return offset < node.header.key_count &&
           std::memcmp(data + d, _get_index_entry_key(node, offset) + d, _header.data_length) == 0;
}

IndexNode &Index::_load_node_for_writing(FileHandle fh, PageOffset node_offset) {
    auto cache_handle = PageManager::instance().load_page({fh, node_offset});
    auto cache = PageManager::instance().access_cache_for_writing(cache_handle);
    return MemoryMapper::map_memory<IndexNode>(cache);
}

const IndexNode &Index::_load_node_for_reading(FileHandle fh, PageOffset node_offset) {
    auto cache_handle = PageManager::instance().load_page({fh, node_offset});
    auto cache = PageManager::instance().access_cache_for_reading(cache_handle);
    return MemoryMapper::map_memory<IndexNode>(cache);
}

IndexEntryOffset Index::search_index_entry(const Byte *data, RecordOffset rid) const {
    if (_header.root_offset == -1) {  // searching in empty tree.
        return {-1, -1};
    }
    auto key_compact = _make_key_compact(data, rid);
    auto result = _search_entry_in(_header.root_offset, key_compact);
    auto &node = _load_node_for_reading(_file_handle, result.page_offset);
    return result.child_offset < node.header.key_count ? result : IndexEntryOffset{-1, -1};
}

IndexEntryOffset Index::next_index_entry_offset(IndexEntryOffset offset) const {
    
    const auto &node = _load_node_for_reading(_file_handle, offset.page_offset);
    if (offset.child_offset + 1 < node.header.key_count) {  // not last
        return {offset.page_offset, offset.child_offset + 1};
    }
    
    // find index entry in next nodes.
    auto next_page = _get_index_node_link(node).next;
    while (next_page != -1) {
        const auto &next_node = _load_node_for_reading(_file_handle, next_page);
        if (next_node.header.key_count != 0) {
            return {next_page, 0};
        }
        next_page = _get_index_node_link(next_node).next;
    }
    
    // the last in the index
    return {-1, -1};
}

IndexEntryOffset Index::prev_index_entry_offset(IndexEntryOffset offset) const {
    
    const auto &node = _load_node_for_reading(_file_handle, offset.page_offset);
    if (offset.child_offset > 0) {  // not first
        return {offset.page_offset, offset.child_offset - 1};
    }
    
    // find index entry in prev nodes.
    auto prev_page = _get_index_node_link(node).prev;
    while (prev_page != -1) {
        const auto &prev_node = _load_node_for_reading(_file_handle, prev_page);
        if (prev_node.header.key_count != 0) {
            return {prev_page, static_cast<ChildOffset>(prev_node.header.key_count - 1)};
        }
        prev_page = _get_index_node_link(prev_node).prev;
    }
    
    // the last in the index
    return {-1, -1};
}

RecordOffset Index::related_record_offset(IndexEntryOffset offset) const {
    if (offset.page_offset < 0 || offset.child_offset < 0) { return {-1, -1}; }
    const auto &node = _load_node_for_reading(_file_handle, offset.page_offset);
    if (offset.child_offset < node.header.key_count) {
        return _get_index_entry_record_offset(node, offset.child_offset);
    }
    return {-1, -1};
}

void Index::insert_index_entry(const Byte *data, RecordOffset rid) {
    
    auto key_compact = _make_key_compact(data, rid);
    if (_header.root_offset == -1) {  // empty, create new root
        auto &root_node = _allocate_index_node();
        _header.root_offset = 1;
        root_node.header.key_count = 1;
        root_node.header.is_leaf = true;
        _write_index_entry_key(root_node, 0, key_compact);
        _write_index_entry_record_offset(root_node, 0, rid);
        _write_index_node_link(root_node, {-1, -1});
    } else {
        static thread_local std::vector<IndexEntryOffset> path;  // stack nodes down the path for split operations.
        path.clear();
        
        for (auto page_offset = _header.root_offset;;) {
            const auto &node = _load_node_for_reading(_file_handle, page_offset);
            auto child_offset = _search_entry_in_node(node, key_compact);
            path.emplace_back(page_offset, child_offset);
            if (node.header.is_leaf) {
                if (child_offset < node.header.key_count && _index_entry_key_matches(node, child_offset, key_compact)) {
                    throw ConflictIndexEntryInsertion{_name};
                }
                break;
            }  // search terminates at leaf.
            page_offset = _get_index_entry_page_offset(node, child_offset);
        }
        
        // Note:
        // Since the height of B+ trees are effectively low, we do not
        // consider the possibility that the page k is bound to will be
        // swapped during the insertion for efficiency, but still this
        // can be thought as a potential vulnerability.
        const auto *k = key_compact;
        auto split_page_offset = 0;
        
        // insert popped up entry into upper nodes.
        while (!path.empty()) {
            
            // fetch the associated node.
            auto e = path.back();
            path.pop_back();
            auto &node = _load_node_for_writing(_file_handle, e.page_offset);
            
            // since the child node was split, the pointer changes.
            auto page_offset = node.header.is_leaf ? rid.page_offset :
                               _get_index_entry_page_offset(node, e.child_offset);  // back-up
            auto slot_offset = node.header.is_leaf ? rid.slot_offset : -1;
            
            if (!node.header.is_leaf) {
                _write_index_entry_page_offset(node, e.child_offset, split_page_offset);
            }
            
            if (node.header.key_count < _header.key_count_per_node) {  // insert directly.
                _move_trailing_index_entries(node, e.child_offset, node, e.child_offset + 1);
                _write_index_entry_key(node, e.child_offset, k);
                _write_index_entry_record_offset(node, e.child_offset, {page_offset, slot_offset});
                node.header.key_count++;
                return;
            }
            
            // allocate page for split node.
            auto split_node_offset = static_cast<PageOffset>(_header.page_count);
            auto &split_node = _allocate_index_node();
            split_node.header.is_leaf = node.header.is_leaf;
            
            // split and insert.
            if (e.child_offset <= _header.key_count_per_node / 2) {  // insert into the former part.
                auto split_pos = _header.key_count_per_node / 2;
                // move entries to split node to make space for insertion.
                _move_trailing_index_entries(node, split_pos, split_node, 0);
                // update entry count.
                split_node.header.key_count = node.header.key_count - split_pos;
                node.header.key_count = split_pos;
                // make space for the new entry.
                _move_trailing_index_entries(node, e.child_offset, node, e.child_offset + 1);
                _write_index_entry_key(node, e.child_offset, k);
                _write_index_entry_record_offset(node, e.child_offset, {page_offset, slot_offset});
                // update entry count.
                node.header.key_count++;
            } else {
                auto split_pos = _header.key_count_per_node / 2 + 1;
                auto split_entry_count = node.header.key_count - split_pos + 1;
                auto split_child_offset = e.child_offset - split_pos;
                _move_trailing_index_entries(node, e.child_offset, split_node, split_child_offset + 1);
                node.header.key_count = static_cast<uint32_t>(e.child_offset);
                _move_trailing_index_entries(node, split_pos, split_node, 0);
                _write_index_entry_key(split_node, split_child_offset, k);
                _write_index_entry_record_offset(split_node, split_child_offset, {page_offset, slot_offset});
                node.header.key_count = split_pos;
                split_node.header.key_count = split_entry_count;
            }
            
            // squeeze
            k = _get_index_entry_key(node, node.header.key_count - 1);
            split_page_offset = split_node_offset;
            if (node.header.is_leaf) {  // link nodes when it is a leaf to form linked-lists.
                auto link = _get_index_node_link(split_node);
                IndexNodeLink leaf_link{link.prev, split_node_offset};
                IndexNodeLink split_link{e.page_offset, link.next};
                _write_index_node_link(node, leaf_link);
                _write_index_node_link(split_node, split_link);
            } else {  // when it is an internal node, just squeeze it.
                node.header.key_count--;
            }
        }
        
        // reach root, new root should be allocate.
        auto root_node_offset = static_cast<PageOffset>(_header.page_count);
        auto &root = _allocate_index_node();
        root.header.key_count = 1;
        _write_index_entry_key(root, 0, k);
        _write_index_entry_page_offset(root, 0, _header.root_offset);
        _write_index_entry_page_offset(root, 1, split_page_offset);
        _header.root_offset = root_node_offset;
    }
}

void Index::delete_index_entry(const Byte *data, RecordOffset rid) {
    if (_header.root_offset == -1) {
        throw EmptyIndexTree{_name};
    }
    auto key_compact = _make_key_compact(data, rid);
    auto entry_offset = _search_entry_in(_header.root_offset, key_compact);
    auto &node = _load_node_for_writing(_file_handle, entry_offset.page_offset);
    if (_index_entry_key_matches(node, entry_offset.child_offset, key_compact)) {
        _move_trailing_index_entries(node, entry_offset.child_offset + 1, node, entry_offset.child_offset);
        node.header.key_count--;
    } else {
        throw IndexEntryNotFound{_name};
    }
}

void Index::delete_index_entry(IndexEntryOffset entry_offset) {
    auto &node = _load_node_for_writing(_file_handle, entry_offset.page_offset);
    if (entry_offset.child_offset < node.header.key_count) {
        _move_trailing_index_entries(node, entry_offset.child_offset + 1, node, entry_offset.child_offset);
        node.header.key_count--;
    } else {
        throw IndexEntryOffsetOutOfRange{_name, entry_offset};
    }
}

RecordOffset Index::search_unique_index_entry(const Byte *data) const {
    
    if (!_header.unique) {
        throw UniqueSearchInNonUniqueIndex{_name};
    }
    
    if (_header.root_offset == -1) {  // searching in empty tree.
        throw EmptyIndexTree{_name};
    }
    
    auto entry_offset = _search_entry_in(_header.root_offset, data);
    IndexNode &node = _load_node_for_writing(_file_handle, entry_offset.page_offset);
    if (_index_entry_key_matches(node, entry_offset.child_offset, data)) {
        return _get_index_entry_record_offset(node, entry_offset.child_offset);
    }
    throw IndexEntryNotFound{_name};
}

bool Index::contains(const Byte *data) const {
    
    if (_header.root_offset == -1) {  // searching in empty tree.
        return false;
    }
    
    auto key_compact = _make_key_compact(data, {-1, -1});
    auto entry_offset = _search_entry_in(_header.root_offset, key_compact);
    auto &node = _load_node_for_writing(_file_handle, entry_offset.page_offset);
    return _index_entry_data_matches(node, entry_offset.child_offset, key_compact);
}

bool Index::data_matches(IndexEntryOffset entry_offset, const Byte *data) const {
    const auto &node = _load_node_for_reading(_file_handle, entry_offset.page_offset);
    auto stored_data = _get_index_entry_key(node, entry_offset.child_offset);
    auto offset = _header.key_length - _header.data_length;
    return std::memcmp(stored_data + offset, data, _header.data_length) == 0;
}

IndexEntryOffset Index::index_entry_offset_begin() const {
    
    auto p = _header.root_offset;
    if (p == -1) {  // searching in empty tree.
        return {-1, -1};
    }
    while (true) {
        auto &node = _load_node_for_reading(_file_handle, p);
        if (node.header.is_leaf) {
            return next_index_entry_offset({p, -1});
        }
        p = _get_index_entry_page_offset(node, 0);
    }
}

bool Index::is_index_entry_offset_end(IndexEntryOffset offset) const {
    return offset == IndexEntryOffset{-1, -1};
}
    
}
