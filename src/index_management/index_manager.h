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

namespace watery {

class IndexManager {

private:
    PageManager &_page_manager{PageManager::instance()};
    std::unordered_map<std::string, std::unordered_map<BufferHandle, BufferOffset>> _used_buffers;

public:
    void create_index(const std::string &name, DataDescriptor key_descriptor);
    void delete_index(const std::string &name);
    Index open_index(const std::string &name);
    void close_index(const Index &index);
    bool is_index_open(const std::string &name) const noexcept;
    
    template<typename Cmp>
    void insert_index_node(const Index &index, const Data &data, RecordOffset record_offset) {
    
    }
    
    template<typename Cmp>
    void delete_index_node(const Index &index, const Data &data, RecordOffset record_offset, Cmp &&comparator) {
    
    }
    
};

}

#endif  // WATERYSQL_INDEX_MANAGER_H
