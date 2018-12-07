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
#include "../data_storage/data.h"
#include "index_entry_offset.h"
#include "index_node_link.h"

namespace watery {

class IndexManager : public Singleton<IndexManager> {

private:
    PageManager &_page_manager{PageManager::instance()};
    std::array<std::unordered_map<BufferHandle, BufferOffset>, MAX_FILE_COUNT> _used_buffers;
    std::unordered_map<std::string, Index> _open_indices;

protected:
    IndexManager() = default;
    
    void _close_index(const Index &index) noexcept;
    
    PageHandle _get_node_page(const Index &index, PageOffset node_offset) noexcept;
    PageHandle _allocate_node_page(Index &index) noexcept;
    
    IndexEntryOffset _search_entry_in(Index &index, PageOffset p, const Byte *data) noexcept;
    
    static ChildOffset _search_entry_in_node(Index &index, const IndexNode &node, const Byte *k) noexcept;
    static uint32_t _get_child_pointer_position(const Index &index, ChildOffset i) noexcept;
    static uint32_t _get_child_key_position(const Index &index, ChildOffset i) noexcept;
    static PageOffset _get_index_entry_page_offset(const Index &index, IndexNode &node, ChildOffset i) noexcept;
    static const Byte *_get_index_entry_key(const Index &index, const IndexNode &node, ChildOffset i) noexcept;
    static RecordOffset _get_index_entry_record_offset(
        const Index &index, const IndexNode &node, ChildOffset i) noexcept;
    static void _write_index_entry_page_offset(const Index &idx, IndexNode &n, ChildOffset i, PageOffset p) noexcept;
    static void _write_index_entry_record_offset(
        const Index &idx, IndexNode &n, ChildOffset i, RecordOffset r) noexcept;
    static void _write_index_entry_key(const Index &idx, IndexNode &n, ChildOffset i, const Byte *k) noexcept;
    static void _write_index_node_link(const Index &idx, IndexNode &n, IndexNodeLink l) noexcept;
    static IndexNodeLink _get_index_node_link(const Index &idx, const IndexNode &n) noexcept;
    static void _move_trailing_index_entries(
        const Index &index, IndexNode &src_node, ChildOffset src_i, IndexNode &dest_node, ChildOffset dest_i) noexcept;
    static IndexNode &_map_index_node_page(const PageHandle &page_handle) noexcept;
    
public:
    ~IndexManager();
    
    void create_index(const std::string &name, DataDescriptor key_descriptor);
    void delete_index(const std::string &name);
    Index &open_index(const std::string &name);
    void close_index(const std::string &index) noexcept;
    void close_all_indices() noexcept;
    bool is_index_open(const std::string &name) const noexcept;
    IndexEntryOffset search_index_entry(Index &index, const Byte *data);
    void insert_index_entry(Index &index, const Byte *data, RecordOffset record_offset);
    
    void delete_index_entry(Index &index, const Byte *data, RecordOffset record_offset);
};

}

#endif  // WATERYSQL_INDEX_MANAGER_H
