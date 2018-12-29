#include <utility>

//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INDEX_H
#define WATERYSQL_INDEX_H

#include <string>
#include "index_header.h"
#include "../config/config.h"
#include "../data/index_key_comparator.h"
#include "index_node.h"
#include "index_node_link.h"
#include "index_entry_offset.h"

namespace watery {

class Index {

private:
    std::string _name;
    FileHandle _file_handle;
    IndexHeader _header;
    IndexKeyComparator _comparator;
    
    IndexEntryOffset _search_entry_in(PageOffset p, const Byte *data) const;
    ChildOffset _search_entry_in_node(const IndexNode &node, const Byte *k) const noexcept;
    
    uint32_t _get_child_pointer_position(ChildOffset i) const noexcept;
    uint32_t _get_child_key_position(ChildOffset i) const noexcept;
    PageOffset _get_index_entry_page_offset(const IndexNode &node, ChildOffset i) const noexcept;
    const Byte *_get_index_entry_key(const IndexNode &node, ChildOffset i) const noexcept;
    RecordOffset _get_index_entry_record_offset(const IndexNode &node, ChildOffset i) const noexcept;
    IndexNodeLink _get_index_node_link(const IndexNode &n) const noexcept;
    
    void _write_index_entry_page_offset(IndexNode &n, ChildOffset i, PageOffset p) noexcept;
    void _write_index_entry_record_offset(IndexNode &n, ChildOffset i, RecordOffset r) noexcept;
    void _write_index_entry_key(IndexNode &n,ChildOffset i,const Byte *k) noexcept;
    void _write_index_node_link(IndexNode &n, IndexNodeLink l) noexcept;
    
    void _move_trailing_index_entries(
        IndexNode &src_node, ChildOffset src_i,IndexNode &dest_node, ChildOffset dest_i) noexcept;
    
    const Byte *_make_key_compact(const Byte *data, RecordOffset rid) const noexcept;
    bool _index_entry_key_matches(const IndexNode &node, ChildOffset offset, const Byte *data) const noexcept;
    bool _index_entry_data_matches(const IndexNode &node, ChildOffset offset, const Byte *data) const noexcept;
    
    IndexNode &_allocate_index_node();
    static IndexNode &_load_node_for_writing(FileHandle fh, PageOffset node_offset);
    static const IndexNode &_load_node_for_reading(FileHandle fh, PageOffset node_offset);

public:
    Index(std::string name, FileHandle fh, IndexHeader h);
    ~Index();
    
    IndexEntryOffset search_index_entry(const Byte *data, RecordOffset rid = {-1, -1}) const;
    IndexEntryOffset next_index_entry_offset(IndexEntryOffset offset) const;
    IndexEntryOffset prev_index_entry_offset(IndexEntryOffset offset) const;
    IndexEntryOffset index_entry_offset_begin() const;
    bool is_index_entry_offset_end(IndexEntryOffset offset) const;
    RecordOffset related_record_offset(IndexEntryOffset offset) const;
    
    void insert_index_entry(const Byte *data, RecordOffset rid);
    void delete_index_entry(const Byte *data, RecordOffset rid);
    void delete_index_entry(IndexEntryOffset entry_offset);
    
    RecordOffset search_unique_index_entry(const Byte *data) const;
    
    bool contains(const Byte *data) const;
    bool data_matches(IndexEntryOffset entry_offset, const Byte *data) const;
    
};

}

#endif  // WATERYSQL_INDEX_H
