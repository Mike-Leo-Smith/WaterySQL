//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_INDEX_MANAGER_H
#define WATERYSQL_INDEX_MANAGER_H

#include "../page_management/page_manager.h"
#include "../data_storage/data_descriptor.h"
#include "index.h"
#include "index_node.h"
#include "../data_storage/data.h"
#include "index_entry_offset.h"
#include "indexNodeLink.h"

namespace watery {

class IndexManager : public Singleton<IndexManager> {
private:
    PageManager &_page_manager{PageManager::instance()};
    std::unordered_map<std::string, std::unordered_map<BufferHandle, BufferOffset>> _used_buffers;

protected:
    IndexManager() = default;
    ~IndexManager() = default;

private:
    PageHandle _get_node_page(const Index &index, PageOffset node_offset);
    PageHandle _allocate_node_page(Index &index);
    
    IndexEntryOffset _search_entry_in(Index &index, PageOffset p, const std::unique_ptr<Data> &data);
    
    static ChildOffset _search_entry_in_node(Index &index, const IndexNode &node, const std::unique_ptr<Data> &k);
    static uint32_t _get_child_pointer_position(const Index &index, ChildOffset i);
    static uint32_t _get_child_key_position(const Index &index, ChildOffset i);
    static PageOffset _get_index_entry_page_offset(const Index &index, IndexNode &node, ChildOffset i);
    static std::unique_ptr<Data> _get_index_entry_key(const Index &index, const IndexNode &node, ChildOffset i);
    static RecordOffset _get_index_entry_record_offset(const Index &index, const IndexNode &node, ChildOffset i);
    
    static void _write_index_entry_page_offset(const Index &idx, IndexNode &n, ChildOffset i, PageOffset p);
    static void _write_index_entry(
        const Index &idx, IndexNode &n, ChildOffset i,
        const std::unique_ptr<Data> &d, RecordOffset &ro);
    static void _write_index_entry_key(const Index &idx, IndexNode &n, ChildOffset i, const std::unique_ptr<Data> &k);
    static void _write_index_node_link(const Index &idx, IndexNode &n, IndexNodeLink l);
    static IndexNodeLink _get_index_node_link(const Index &idx, const IndexNode &n);
    static void _move_trailing_index_entries(
        const Index &index, IndexNode &src_node, ChildOffset src_i, IndexNode &dest_node, ChildOffset dest_i);

public:
    void create_index(const std::string &name, DataDescriptor key_descriptor);
    void delete_index(const std::string &name);
    Index open_index(const std::string &name);
    void close_index(const Index &index);
    bool is_index_open(const std::string &name) const noexcept;
    
    IndexEntryOffset search_index_entry(Index &index, const std::unique_ptr<Data> &data);
    void insert_index_entry(Index &index, const std::unique_ptr<Data> &data, RecordOffset record_offset);
    void delete_index_entry(Index &index, const std::unique_ptr<Data> &data, RecordOffset record_offset);
    
    IndexNode &_map_index_node_page(const PageHandle &page_handle) const;
};

}

#endif  // WATERYSQL_INDEX_MANAGER_H
