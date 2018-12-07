//
// Created by Mike Smith on 2018-12-02.
//

#ifndef WATERYSQL_SYSTEM_MANAGER_H
#define WATERYSQL_SYSTEM_MANAGER_H

#include <filesystem>
#include <iostream>
#include <unordered_set>

#include "../utility/type_constraints/singleton.h"

namespace watery {

class SystemManager : public Singleton<SystemManager> {

private:
    std::filesystem::path _base_path{std::filesystem::current_path() / "watery-databases"};
    std::unordered_set<std::string> _existing_databases;
    std::string _current_database;

protected:
    SystemManager() = default;

public:
    void create_database(const std::string &name);
    void delete_database(const std::string &name);
    void use_database(const std::string &name);
    const std::vector<std::string> &all_databases() const;
};

}

#endif  // WATERYSQL_SYSTEM_MANAGER_H
