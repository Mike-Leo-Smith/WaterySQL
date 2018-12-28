#include <utility>

#include <utility>

//
// Created by Mike Smith on 2018/11/24.
//

#include <cstring>

#include "index_manager.h"
#include "index_entry_offset.h"
#include "../error/page_manager_error.h"
#include "../utility/io/error_printer.h"
#include "../utility/memory/memory_mapper.h"
#include "../utility/mathematics/sgn.h"
#include "../error/closing_shared_index.h"
#include "../error/index_entry_oversized.h"

namespace watery {

void IndexManager::create_index(const std::string &name, DataDescriptor data_desc, bool unique) {
    
    auto dl = data_desc.length;
    auto kl = static_cast<uint32_t>(unique ? dl : dl + sizeof(RecordOffset));
    auto pl = static_cast<uint32_t>(sizeof(RecordOffset));
    auto cpn = (PAGE_SIZE - sizeof(IndexNodeHeader) - 8 /* for alignment */) / (kl + pl) / 2 * 2;  // rounded to even
    
    if (cpn == 0) {
        throw IndexEntryOversized{name, kl};
    }
    
    auto file_name = name + INDEX_FILE_EXTENSION;
    PageManager::instance().create_file(file_name);
    auto file_handle = PageManager::instance().open_file(file_name);
    
    auto cache_handle = PageManager::instance().allocate_page({file_handle, 0});
    auto cache = PageManager::instance().access_cache_for_writing(cache_handle);
    MemoryMapper::map_memory<IndexHeader>(cache) = {data_desc, unique, kl, dl, 1, static_cast<uint32_t>(cpn), -1};
    PageManager::instance().close_file(file_handle);
}

void IndexManager::delete_index(std::string name) {
    close_index(name);
    PageManager::instance().delete_file(name.append(INDEX_FILE_EXTENSION));
}

std::shared_ptr<Index> IndexManager::open_index(const std::string &name) {
    if (_open_indices.count(name) == 0) {
        FileHandle file_handle = PageManager::instance().open_file(name + INDEX_FILE_EXTENSION);
        auto cache_handle = PageManager::instance().load_page({file_handle, 0});
        auto cache = PageManager::instance().access_cache_for_reading(cache_handle);
        const auto &index_header = MemoryMapper::map_memory<IndexHeader>(cache);
        _open_indices.emplace(name, std::make_shared<Index>(name, file_handle, index_header));
    }
    return _open_indices[name];
}

void IndexManager::close_index(const std::string &name) {
    if (auto it = _open_indices.find(name); it != _open_indices.end()) {
        if (!it->second.unique()) {
            throw ClosingSharedIndex{name};
        }
        _open_indices.erase(it);
    }
}

IndexManager::~IndexManager() {
    _open_indices.clear();
}

void IndexManager::close_all_indices() {
    for (auto &&entry : _open_indices) {
        if (!entry.second.unique()) {
            throw ClosingSharedIndex{entry.first};
        }
    }
    _open_indices.clear();
}

}
