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
#include "index_node_pointer_offset.h"

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
    
    void _split_and_insert(Index &index, IndexEntryOffset e, const Data &k, RecordOffset r);
    IndexEntryOffset _search_below(Index &index, PageOffset node_offset, const Data &data);
    
    static uint32_t _get_child_pointer_offset(const Index &index, ChildOffset i);
    static uint32_t _get_child_key_offset(const Index &index, ChildOffset i);
    static PageOffset _get_child_page_offset(const Index &index, IndexNode &node, ChildOffset i);
    static std::unique_ptr<Data> _get_index_entry_key(const Index &index, const IndexNode &node, ChildOffset i);
    static RecordOffset _get_index_entry_record_offset(const Index &index, const IndexNode &node, ChildOffset i);
    
    static void _write_index_entry(const Index &idx, IndexNode &n, ChildOffset i, const Data &d, RecordOffset &ro);
    static void _move_trailing_index_entries(
        const Index &index, IndexNode &src_node, ChildOffset src_i, IndexNode &dest_node, ChildOffset dest_i);

public:
    void create_index(const std::string &name, DataDescriptor key_descriptor);
    void delete_index(const std::string &name);
    Index open_index(const std::string &name);
    void close_index(const Index &index);
    bool is_index_open(const std::string &name) const noexcept;
    
    IndexEntryOffset search_index_entry(Index &index, const Data &data);
    void insert_index_entry(Index &index, const Data &data, RecordOffset record_offset);
    void delete_index_entry(Index &index, const Data &data, RecordOffset record_offset);
    
    IndexNode &map_index_node_page(const PageHandle &page_handle) const;
};

}

#endif  // WATERYSQL_INDEX_MANAGER_H
