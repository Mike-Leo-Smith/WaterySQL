//
// Created by Mike Smith on 2018-12-02.
//

#ifndef WATERYSQL_SYSTEM_MANAGER_H
#define WATERYSQL_SYSTEM_MANAGER_H

#include <filesystem>
#include <iostream>
#include <unordered_set>

#include "../utility/type_constraints/singleton.h"
#include "../record_management/record_manager.h"
#include "../index_management/index_manager.h"

namespace watery {

class SystemManager : public Singleton<SystemManager> {

private:
    RecordManager &_record_manager = RecordManager::instance();
    IndexManager &_index_manager = IndexManager::instance();
    std::filesystem::path _base_path{std::filesystem::current_path() / DATABASE_BASE_PATH};
    std::string _current_database;

protected:
    SystemManager() = default;

public:
    void create_database(const std::string &name);
    void delete_database(const std::string &name);
    void use_database(const std::string &name);
    const std::vector<std::string> &all_databases() const;
    const std::vector<std::string> &all_tables() const;
};

}

#endif  // WATERYSQL_SYSTEM_MANAGER_H
