//
// Created by Mike Smith on 2018-12-02.
//

#include <filesystem>
#include <fstream>
#include <vector>

#include "system_manager.h"
#include "../error/system_manager_error.h"
#include "../utility/io/error_printer.h"
#include "../query/query_engine.h"

namespace watery {

void SystemManager::create_database(const std::string &name) {
    
    if (!std::filesystem::exists(_base_path) && !std::filesystem::create_directories(_base_path)) {
        throw SystemManagerError{"Failed to create directory for holding databases."};
    }
    
    auto dir_name = name + DATABASE_DIRECTORY_EXTENSION;
    if (_database_list.count(name) != 0 || std::filesystem::exists(_base_path / dir_name)) {
        throw SystemManagerError{
            std::string{"Failed to create database \""}.append(name).append("\" ").append("which already exists.")};
    }
    try {
        std::filesystem::create_directories(_base_path / dir_name);
    } catch (const std::filesystem::filesystem_error &e) {
        throw SystemManagerError{std::string{"Failed to create database \""}.append(name).append("\".")};
    }
    _database_list.emplace(name);
}

void SystemManager::drop_database(const std::string &name) {
    
    if (name == _current_database) {
        RecordManager::instance().close_all_tables();
        IndexManager::instance().close_all_indexes();
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
    
    RecordManager::instance().close_all_tables();
    IndexManager::instance().close_all_indexes();
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

void SystemManager::create_table(const std::string &name, RecordDescriptor descriptor) {
    
    if (_current_database.empty()) {
        throw SystemManagerError{"No database currently in use."};
    }
    
    if (_table_list.count(name) != 0) {
        throw SystemManagerError{
            std::string{"Failed to create table \""}.append(name).append("\" which already exists.")};
    }
    
    static thread_local std::vector<std::string> indexes;
    static thread_local std::vector<std::string> foreign_tables;
    
    indexes.clear();
    foreign_tables.clear();
    bool table_created = false;
    
    try {
        for (auto i = 0; i < descriptor.field_count; i++) {
            auto &&field_desc = descriptor.field_descriptors[i];
            if (field_desc.constraints.foreign()) {
                std::string foreign_table_name{field_desc.foreign_table_name.data()};
                std::string foreign_column_name{field_desc.foreign_column_name.data()};
                _visit_field_descriptor(  // check if the foreign column exists and is primary key
                    foreign_table_name,
                    foreign_column_name,
                    [&name](const FieldDescriptor &fd) {
                        if (!fd.constraints.primary()) {
                            throw SystemManagerError{
                                std::string{"Failed to create table \""}.append(name).append(
                                    "\" which has a foreign key constraint referencing a non-primary key column.")};
                        }
                    });
                auto foreign_table = RecordManager::instance().open_table(foreign_table_name);
                foreign_table->add_foreign_key_reference(name);
                foreign_tables.emplace_back(std::move(foreign_table_name));
            }
            if (field_desc.constraints.unique()) {
                auto index_name = std::string{name}.append(".").append(field_desc.name.data());
                IndexManager::instance().create_index(index_name, field_desc.data_descriptor, true);
                field_desc.indexed = true;
                indexes.emplace_back(index_name);
            }
        }
        // now actually able to create the table
        RecordManager::instance().create_table(name, descriptor);
        table_created = true;
        _table_list.emplace(name);
    } catch (...) {  // assumed that no more exceptions are thrown during rollback
        auto e = std::current_exception();
        if (table_created) {
            RecordManager::instance().delete_table(name);
        }
        for (auto &&ft : foreign_tables) {
            RecordManager::instance().open_table(ft)->drop_foreign_key_reference(name);
        }
        for (auto &&idx : indexes) {
            IndexManager::instance().delete_index(idx);
        }
        std::rethrow_exception(e);  // rethrow
    }
}

void SystemManager::drop_table(const std::string &name) {
    if (_table_list.count(name) == 0) {
        throw SystemManagerError{
            std::string{"Failed to drop table \""}.append(name).append("\" which does not exists.")};
    }
    
    {
        auto table = RecordManager::instance().open_table(name);
        if (table->foreign_key_reference_count() != 0) {
            throw SystemManagerError{
                std::string{"Failed to drop table \""}.append(name).append("\" which is referenced by other tables.")};
        }
        
        auto &&record_desc = table->descriptor();
        if (record_desc.foreign_referencing) {  // drop foreign record ref counts
            for (auto rid = table->record_offset_begin();
                 !table->is_record_offset_end(rid);
                 rid = table->next_record_offset(rid)) {
                for (auto i = 0; i < record_desc.field_count; i++) {
                    auto &&field_desc = record_desc.field_descriptors[i];
                    if (!field_desc.constraints.foreign()) {
                        continue;
                    }
                    auto rec = table->get_record(rid);
                    if (field_desc.constraints.nullable() &&
                        MemoryMapper::map_memory<NullFieldBitmap>(rec)[i]) {
                        continue;
                    }
                    std::string foreign_name{field_desc.foreign_table_name.data()};
                    auto field = rec + record_desc.field_offsets[i];
                    auto foreign_table = RecordManager::instance().open_table(foreign_name);
                    foreign_name.append(".").append(field_desc.foreign_column_name.data());
                    auto foreign_index = IndexManager::instance().open_index(foreign_name);
                    auto foreign_rid = foreign_index->search_unique_index_entry(field);
                    foreign_table->drop_record_reference_count(foreign_rid);
                }
            }
        }
        
        for (auto i = 0; i < record_desc.field_count; i++) {
            auto &&field_desc = record_desc.field_descriptors[i];
            if (field_desc.indexed) {
                IndexManager::instance().delete_index(std::string{name}.append(".").append(field_desc.name.data()));
            }
            if (field_desc.constraints.foreign()) {
                auto foreign_table = RecordManager::instance().open_table(field_desc.foreign_table_name.data());
                foreign_table->drop_foreign_key_reference(table->name());
            }
        }
    }
    RecordManager::instance().delete_table(name);
    _table_list.erase(name);
}

void SystemManager::create_index(const std::string &table_name, const std::string &column_name) {
    
    if (_current_database.empty()) {
        throw SystemManagerError{"No database currently in use."};
    }
    
    auto table = RecordManager::instance().open_table(table_name);
    auto &desc = table->descriptor();
    auto cid = table->column_offset(column_name);
    auto &field_desc = desc.field_descriptors[cid];
    
    if (field_desc.indexed) {
        throw SystemManagerError{
            std::string{"Failed to create index for column \""}
                .append(column_name).append("\" in table \"").append(table_name)
                .append("\" which is already indexed.")};
    }
    auto index_name = (table_name + ".").append(column_name);
    IndexManager::instance().create_index(index_name, field_desc.data_descriptor, field_desc.constraints.unique());
    field_desc.indexed = true;
    
    auto index = IndexManager::instance().open_index(index_name);
    
    auto rid = table->record_offset_begin();
    while (!table->is_record_offset_end(rid)) {
        auto rec = table->get_record(rid);
        index->insert_index_entry(rec + desc.field_offsets[cid], rid);
        rid = table->next_record_offset(rid);
    }
    
}

void SystemManager::drop_index(const std::string &table_name, const std::string &column_name) {
    _visit_field_descriptor(
        table_name,
        column_name,
        [&tn = table_name, &cn = column_name, &im = IndexManager::instance()](FieldDescriptor &fd) {
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

const RecordDescriptor &SystemManager::describe_table(const std::string &table_name) {
    return RecordManager::instance().open_table(table_name)->descriptor();
}

void SystemManager::finish() {
    commit();
    _table_list.clear();
    _current_database = "";
}

void SystemManager::commit() {
    IndexManager::instance().close_all_indexes();
    RecordManager::instance().close_all_tables();
}

const std::string &SystemManager::current_database() const noexcept {
    return _current_database;
}

}
