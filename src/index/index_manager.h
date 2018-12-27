//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_INDEX_MANAGER_H
#define WATERYSQL_INDEX_MANAGER_H

#include <string>
#include <unordered_map>
#include <memory>
#include <array>
#include "../page/page_manager.h"
#include "../data/data_descriptor.h"
#include "index.h"
#include "index_node.h"
#include "index_entry_offset.h"
#include "index_node_link.h"
#include "../data/field_descriptor.h"

namespace watery {

class IndexManager : public Singleton<IndexManager> {

private:
    PageManager &_page_manager{PageManager::instance()};
    std::map<std::string, std::shared_ptr<Index>> _open_indices;

protected:
    IndexManager() = default;
    
    void _close_index(const std::shared_ptr<Index> &index) noexcept;
    
    IndexNode & _allocate_node(const std::shared_ptr<Index> &index) noexcept;
    
    IndexEntryOffset _search_entry_in(const std::shared_ptr<Index> &index, PageOffset p, const Byte *data) noexcept;
    
    static ChildOffset _search_entry_in_node(
        const std::shared_ptr<Index> &index, const IndexNode &node, const Byte *k) noexcept;
    static uint32_t _get_child_pointer_position(const std::shared_ptr<Index> &index, ChildOffset i) noexcept;
    static uint32_t _get_child_key_position(const std::shared_ptr<Index> &index, ChildOffset i) noexcept;
    static PageOffset _get_index_entry_page_offset(
        const std::shared_ptr<Index> &index, const IndexNode &node, ChildOffset i) noexcept;
    static const Byte *_get_index_entry_key(
        const std::shared_ptr<Index> &index, const IndexNode &node, ChildOffset i) noexcept;
    static RecordOffset _get_index_entry_record_offset(
        const std::shared_ptr<Index> &index, const IndexNode &node, ChildOffset i) noexcept;
    static void _write_index_entry_page_offset(
        const std::shared_ptr<Index> &idx, IndexNode &n,
        ChildOffset i, PageOffset p) noexcept;
    static void _write_index_entry_record_offset(
        const std::shared_ptr<Index> &idx, IndexNode &n, ChildOffset i, RecordOffset r) noexcept;
    static void _write_index_entry_key(const std::shared_ptr<Index> &idx,
                                          IndexNode &n,
                                          ChildOffset i,
                                          const Byte *k) noexcept;
    static void _write_index_node_link(const std::shared_ptr<Index> &idx, IndexNode &n, IndexNodeLink l) noexcept;
    static IndexNodeLink _get_index_node_link(const std::shared_ptr<Index> &idx, const IndexNode &n) noexcept;
    static void _move_trailing_index_entries(
        const std::shared_ptr<Index> &index, IndexNode &src_node, ChildOffset src_i,
        IndexNode &dest_node, ChildOffset dest_i) noexcept;
    static const Byte * _make_key_compact(const std::shared_ptr<Index> &index, const Byte *data, RecordOffset rid);
    
    static std::shared_ptr<Index> _try_lock_index_weak_pointer(std::weak_ptr<Index> idx_ptr);
    
    static bool _index_entry_key_matches(
        const std::shared_ptr<Index> &index, const IndexNode &node, ChildOffset offset, const Byte *data);
    
    static bool _index_entry_data_matches(
        const std::shared_ptr<Index> &index, const IndexNode &node, ChildOffset offset, const Byte *data);
    
    IndexNode &_load_node_for_writing(FileHandle fh, PageOffset node_offset);
    const IndexNode &_load_node_for_reading(FileHandle fh, PageOffset node_offset);
    IndexHeader &_load_index_header_for_writing(FileHandle fh);
    const IndexHeader &_load_index_header_for_reading(FileHandle fh);
    IndexHeader &_allocate_index_header(FileHandle fh);

public:
    ~IndexManager();
    
    void create_index(const std::string &name, DataDescriptor data_desc, bool unique);
    void delete_index(const std::string &name);
    std::weak_ptr<Index> open_index(const std::string &name);
    void close_index(const std::string &name) noexcept;
    void close_all_indices() noexcept;
    bool is_index_open(const std::string &name) const noexcept;
    
    IndexEntryOffset search_index_entry(std::weak_ptr<Index> index, const Byte *data);
    IndexEntryOffset next_index_entry_offset(std::weak_ptr<Index> index, IndexEntryOffset offset);
    IndexEntryOffset prev_index_entry_offset(std::weak_ptr<Index> index, IndexEntryOffset offset);
    RecordOffset related_record_offset(std::weak_ptr<Index> index, IndexEntryOffset offset);
    
    void insert_index_entry(std::weak_ptr<Index> index, const Byte *data, RecordOffset rid);
    
    void delete_index_entry(std::weak_ptr<Index> index, const Byte *data, RecordOffset rid);
    void delete_index_entry(std::weak_ptr<Index> index, IndexEntryOffset entry_offset);
    
    RecordOffset search_unique_index_entry(std::weak_ptr<Index> index,const Byte *data);
    
    bool contains(std::weak_ptr<Index> index, const Byte *data);
    bool data_matches(std::weak_ptr<Index> index, IndexEntryOffset entry_offset, const Byte *data);
    
};

}

#endif  // WATERYSQL_INDEX_MANAGER_H
