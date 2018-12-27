#include <utility>

#include <utility>

//
// Created by Mike Smith on 2018/11/24.
//

#include <cstring>

#include "record_manager.h"
#include "data_page.h"
#include "../error/page_manager_error.h"
#include "../utility/io/error_printer.h"
#include "../utility/memory/memory_mapper.h"
#include "../error/record_manager_error.h"

namespace watery {

void RecordManager::create_table(const std::string &name, const RecordDescriptor &record_descriptor) {
    
    auto rl = record_descriptor.length;
    auto spp = std::min(MAX_SLOT_COUNT_PER_PAGE,
                        static_cast<uint32_t>((PAGE_SIZE - sizeof(DataPageHeader) - 8 /* for alignment */) / rl));
    
    if (spp == 0) {
        throw RecordManagerError{
            std::string{"Failed to create table because the records are too long ("}
                .append(std::to_string(rl)).append(" bytes).")};
    }
    
    auto file_name = name + TABLE_FILE_EXTENSION;
    
    _page_manager.create_file(file_name);
    auto file_handle = _page_manager.open_file(file_name);
    
    auto cache_handle = _page_manager.allocate_page({file_handle, 0});
    auto cache = _page_manager.access_cache_for_writing(cache_handle);
    MemoryMapper::map_memory<TableHeader>(cache) = {record_descriptor, 1, 0, rl, spp, -1};
    _page_manager.close_file(file_handle);
}

std::weak_ptr<Table> RecordManager::open_table(const std::string &name) {
    
    if (_open_tables.count(name) == 0) {
        
        FileHandle file_handle = _page_manager.open_file(name + TABLE_FILE_EXTENSION);
        
        // load table header
        auto cache_handle = _page_manager.load_page({file_handle, 0});
        auto cache = _page_manager.access_cache_for_reading(cache_handle);
        const auto &table_header = MemoryMapper::map_memory<TableHeader>(cache);
        _open_tables.emplace(name, std::make_shared<Table>(name, file_handle, table_header));
    }
    
    return _open_tables[name];
}

void RecordManager::close_table(const std::string &name) {
    _open_tables.erase(name);
}

void RecordManager::delete_table(std::string name) {
    close_table(name);
    _page_manager.delete_file(name.append(TABLE_FILE_EXTENSION));
}

void RecordManager::close_all_tables() {
    _open_tables.clear();
}

RecordManager::~RecordManager() {
    close_all_tables();
}

}
