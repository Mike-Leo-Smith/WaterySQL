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

public:
    ~IndexManager();
    
    void create_index(const std::string &name, DataDescriptor data_desc, bool unique);
    void delete_index(std::string name);
    std::shared_ptr<Index> open_index(const std::string &name);
    void close_index(const std::string &name) noexcept;
    void close_all_indices() noexcept;
};

}

#endif  // WATERYSQL_INDEX_MANAGER_H
