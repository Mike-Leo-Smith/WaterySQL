//
// Created by Mike Smith on 2018-12-02.
//

#include <filesystem>
#include <fstream>
#include <vector>

#include "system_manager.h"
#include "../errors/system_manager_error.h"
#include "../utility/io/error_printer.h"

namespace watery {

void SystemManager::create_database(const std::string &name) {
    auto dir_name = name + DATABASE_DIRECTORY_EXTENSION;
    if (_database_list.count(name) != 0 || std::filesystem::exists(_base_path / dir_name)) {
        throw SystemManagerError{
            std::string{"Failed to create database \""}.append(dir_name).append("\" ").append("which already exists.")};
    }
    try {
        std::filesystem::create_directories(_base_path / dir_name);
    } catch (const std::filesystem::filesystem_error &e) {
        throw SystemManagerError{std::string{"Failed to create database \""}.append(dir_name).append("\".")};
    }
    _database_list.emplace(name);
}

void SystemManager::drop_database(const std::string &name) {
    
    if (name == _current_database) {
        _record_manager.close_all_tables();
        _index_manager.close_all_indices();
        _table_list.clear();
        _current_database = "";
    }
    
    auto dir_name = name + DATABASE_DIRECTORY_EXTENSION;
    if (_database_list.count(name) == 0 || !std::filesystem::exists(_base_path / dir_name)) {
        throw SystemManagerError{
            std::string{"Failed to drop database \""}.append(name).append("\" which does not exist.")};
    }
    try {
        std::filesystem::current_path(_base_path);
        _current_database = "";
    } catch (const std::filesystem::filesystem_error &e) {
        throw SystemManagerError{
            std::string{"Failed to remove database \""}
                .append(name).append("\"")
                .append(". Cannot move out of the database directory.")};
    }
    if (std::filesystem::remove_all(_base_path / dir_name) == 0) {
        throw SystemManagerError{std::string{"Failed to remove database \""}.append(name).append("\".")};
    }
    _database_list.erase(name);
}

void SystemManager::use_database(const std::string &name) {
    
    if (name == _current_database) {
        return;
    }
    
    auto dir_name = name + DATABASE_DIRECTORY_EXTENSION;
    if (_database_list.count(name) == 0 || !std::filesystem::exists(_base_path / dir_name)) {
        throw SystemManagerError{
            std::string{"Failed to use database \""}.append(name).append("\" which does not exist.")};
    }
    try {
        std::filesystem::current_path(_base_path / dir_name);
        _current_database = name;
    } catch (const std::filesystem::filesystem_error &e) {
        throw SystemManagerError{std::string{"Failed to use database \""}.append(name).append("\".")};
    }
    
    _record_manager.close_all_tables();
    _index_manager.close_all_indices();
    _scan_tables();
}

const std::set<std::string> &SystemManager::database_list() const {
    return _database_list;
}

const std::set<std::string> &SystemManager::table_list() const {
    if (_current_database.empty()) {
        throw SystemManagerError{"Failed to list tables because no database is currently in use."};
    }
    return _table_list;
}

void SystemManager::create_table(const std::string &name, const RecordDescriptor &descriptor) {

}

void SystemManager::drop_table(const std::string &name) {
    if (_table_list.count(name) == 0) {
        throw SystemManagerError{
            std::string{"Failed to drop table \""}.append(name).append("\" which does not exists.")};
    }
    auto &&table_header = _record_manager.open_table(name).header;
    if (table_header.foreign_key_reference_count != 0) {
        throw SystemManagerError{
            std::string{"Failed to drop table \""}.append(name).append("\" which is referenced by other tables.")};
    }
    
    auto &&record_desc = table_header.record_descriptor;
    for (auto i = 0; i < record_desc.field_count; i++) {
        auto &&field_desc = record_desc.field_descriptors[i];
        if (field_desc.indexed) {
            try {
                _index_manager.delete_index(std::string{name}.append(".").append(field_desc.name.data()));
            } catch (const std::exception &e) {
                print_error(std::cerr, e);
            }
        }
        if (field_desc.constraints.foreign()) {
            try {
                _record_manager.open_table(field_desc.foreign_table_name.data()).header.foreign_key_reference_count--;
            } catch (const std::exception &e) {
                print_error(std::cerr, e);
            }
        }
    }
    _record_manager.delete_table(name);
    _table_list.erase(name);
}

void SystemManager::create_index(const std::string &table_name, const std::string &column_name) {
    _visit_field_descriptor(
        table_name, column_name, [&tn = table_name, &cn = column_name, &im = _index_manager](FieldDescriptor &fd) {
            if (fd.indexed) {
                throw SystemManagerError{
                    std::string{"Failed to create index for column \""}
                        .append(cn).append("\" in table \"").append(tn).append("\" which is already indexed.")};
            }
            im.create_index(std::string{tn}.append(".").append(cn), fd.data_descriptor, fd.constraints.unique());
            fd.indexed = true;
        });
}

void SystemManager::drop_index(const std::string &table_name, const std::string &column_name) {
    _visit_field_descriptor(
        table_name, column_name, [&tn = table_name, &cn = column_name, &im = _index_manager](FieldDescriptor &fd) {
            if (fd.constraints.unique()) {
                throw SystemManagerError{
                    std::string{"Failed to drop index for column \""}
                        .append(cn).append("\" in table \"").append(tn)
                        .append("\" which is under UNIQUE KEY constraint.")};
            }
            if (!fd.indexed) {
                throw SystemManagerError{
                    std::string{"Failed to drop index for column \""}
                        .append(cn).append("\" in table \"").append(tn).append("\" which is not indexed.")};
            }
            im.delete_index(std::string{tn}.append(".").append(cn));
            fd.indexed = false;
        });
}

void SystemManager::_scan_databases() {
    _database_list.clear();
    for (auto &&database: std::filesystem::directory_iterator{_base_path}) {
        if (database.path().extension() == DATABASE_DIRECTORY_EXTENSION) {
            _database_list.emplace(database.path().stem());
        }
    }
}

void SystemManager::_scan_tables() {
    _table_list.clear();
    if (!_current_database.empty()) {
        auto curr_db_dir = _current_database + DATABASE_DIRECTORY_EXTENSION;
        for (auto &&table: std::filesystem::directory_iterator{_base_path / curr_db_dir}) {
            if (table.path().extension() == TABLE_FILE_EXTENSION) {
                _table_list.emplace(table.path().stem());
            }
        }
    }
}

SystemManager::SystemManager() {
    _scan_databases();
}

}
