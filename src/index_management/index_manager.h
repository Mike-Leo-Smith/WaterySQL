//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_INDEX_MANAGER_H
#define WATERYSQL_INDEX_MANAGER_H

#include <string>
#include <unordered_map>
#include <array>
#include "../page_management/page_manager.h"
#include "../data_storage/data_descriptor.h"
#include "index.h"
#include "index_node.h"
#include "index_entry_offset.h"
#include "index_node_link.h"
#include "../data_storage/field_descriptor.h"

namespace watery {

class IndexManager : public Singleton<IndexManager> {

private:
    FileManager &_page_manager{FileManager::instance()};
    std::unordered_map<std::string, Index> _open_indices;

protected:
    IndexManager() = default;
    
    void _close_index(const Index &index) noexcept;
    
    CacheHandle _load_node_page_cache(const Index &index, PageOffset node_offset) noexcept;
    CacheHandle _allocate_node_page_cache(Index &index) noexcept;
    
    IndexEntryOffset _search_entry_in(Index &index, PageOffset p, const Byte *data) noexcept;
    
    static ChildOffset _search_entry_in_node(Index &index, const IndexNode &node, const Byte *k) noexcept;
    static uint32_t _get_child_pointer_position(const Index &index, ChildOffset i) noexcept;
    static uint32_t _get_child_key_position(const Index &index, ChildOffset i) noexcept;
    static PageOffset _get_index_entry_page_offset(const Index &index, const IndexNode &node, ChildOffset i) noexcept;
    static const Byte *_get_index_entry_key(const Index &index, const IndexNode &node, ChildOffset i) noexcept;
    static RecordOffset _get_index_entry_record_offset(
        const Index &index, const IndexNode &node, ChildOffset i) noexcept;
    static void _write_index_entry_page_offset(const Index &idx, IndexNode &n, ChildOffset i, PageOffset p) noexcept;
    static void _write_index_entry_record_offset(
        const Index &idx, IndexNode &n, ChildOffset i, RecordOffset r) noexcept;
    static void _write_index_entry_key(
        const Index &idx, IndexNode &n, ChildOffset i, const Byte *k, RecordOffset rid) noexcept;
    static void _write_index_node_link(const Index &idx, IndexNode &n, IndexNodeLink l) noexcept;
    static IndexNodeLink _get_index_node_link(const Index &idx, const IndexNode &n) noexcept;
    static void _move_trailing_index_entries(
        const Index &index, IndexNode &src_node, ChildOffset src_i, IndexNode &dest_node, ChildOffset dest_i) noexcept;

public:
    ~IndexManager();
    
    void create_index(const std::string &name, DataDescriptor data_desc, bool unique);
    void delete_index(const std::string &name);
    Index &open_index(const std::string &name);
    void close_index(const std::string &index) noexcept;
    void close_all_indices() noexcept;
    bool is_index_open(const std::string &name) const noexcept;
    IndexEntryOffset search_index_entry(Index &index, const Byte *data);
    void insert_index_entry(Index &index, const Byte *data, RecordOffset rid);
    
    void delete_index_entry(Index &index, const Byte *data, RecordOffset rid);
    void _make_key_compact(const Index &index, std::vector<Byte> &key_compact, const Byte *data, RecordOffset rid) const;
};

}

#endif  // WATERYSQL_INDEX_MANAGER_H
