//
// Created by Mike Smith on 2018/11/4.
//

#include <filesystem>

#include "record_manager.h"

namespace watery {

void RecordManager::create_table(const std::string &name, const TableDescriptor &descriptor) {
    
    auto file_name = name + ".table";
    if (std::filesystem::exists(file_name)) {
        std::cerr << "Failed to create table \"" << name << "\": file with the same name already exists." << std::endl;
        return;
    }
    
    if (!_file_manager.createFile(file_name.c_str())) {
        std::cerr << "Failed to create table \"" << name << "\": file cannot be created." << std::endl;
        return;
    }
    
    auto file_id = 0;
    if (!_file_manager.openFile(file_name.c_str(), file_id)) {
        std::cerr << "Failed to create table \"" << name << "\": file cannot be opened." << std::endl;
        return;
    }
    
    auto buffer_id = 0;
    
    // write header to the first page.
    auto page = _page_manager.allocPage(file_id, 0, buffer_id);
    *reinterpret_cast<TableDescriptor *>(page) = descriptor;
    _page_manager.markDirty(buffer_id);
    _page_manager.writeBack(buffer_id);
    _file_manager.closeFile(file_id);
}

std::optional<Table> RecordManager::open_table(const std::string &name) {
    
    auto file_name = name + ".table";
    auto file_id = 0;
    if (!_file_manager.openFile(file_name.c_str(), file_id)) {
        std::cout << "Failed to open table \"" << name << "\": file cannot be opened." << std::endl;
        return std::nullopt;
    }
    
    auto buffer_id = 0;
    auto page = _page_manager.getPage(file_id, 0, buffer_id);
    _page_manager.markAccess(buffer_id);
    auto descriptor = *reinterpret_cast<TableDescriptor *>(page);
    
    return Table{name, descriptor, file_id};
}

void RecordManager::close_table(int32_t id) {

}

void RecordManager::delete_table(const std::string &name) {

}

void RecordManager::insert_record(const Table &table, const Record &record) {

}

void RecordManager::update_record(const Table &table, const Record &record) {

}

void RecordManager::delete_record(const Table &table, int32_t slot) {

}

}
