//
// Created by Mike Smith on 2018-12-02.
//

#include <filesystem>
#include <fstream>
#include <vector>

#include "system_manager.h"
#include "../errors/system_manager_error.h"

namespace watery {

void SystemManager::create_database(const std::string &name) {
    auto dir_name = name + ".db";
    if (std::filesystem::exists(_base_path / dir_name)) {
        throw SystemManagerError{
            std::string{"Failed to create database \""}.append(dir_name).append("\" ").append("which already exists.")};
    }
    try {
        std::filesystem::create_directories(_base_path / dir_name);
    } catch (const std::filesystem::filesystem_error &e) {
        throw SystemManagerError{std::string{"Failed to create database \""}.append(dir_name).append("\".")};
    }
}

void SystemManager::delete_database(const std::string &name) {
    auto dir_name = name + ".db";
    if (!std::filesystem::exists(_base_path / dir_name)) {
        return;
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
}

void SystemManager::use_database(const std::string &name) {
    auto dir_name = name + ".db";
    if (!std::filesystem::exists(_base_path / dir_name)) {
        throw SystemManagerError{
            std::string{"Failed to use database \""}.append(name).append("\" which does not exist.")};
    }
    try {
        std::filesystem::current_path(_base_path / dir_name);
        _current_database = name;
    } catch (const std::filesystem::filesystem_error &e) {
        throw SystemManagerError{std::string{"Failed to use database \""}.append(name).append("\".")};
    }
}

const std::vector<std::string> &SystemManager::all_databases() const {
    thread_local static std::vector<std::string> databases;
    
    databases.clear();
    for (auto &&dir: std::filesystem::directory_iterator{_base_path}) {
        if (dir.path().extension() == ".db") {
            databases.emplace_back(dir.path().filename().stem());
        }
    }
    
    return databases;
}

}
