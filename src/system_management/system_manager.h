//
// Created by Mike Smith on 2018-12-02.
//

#ifndef WATERYSQL_SYSTEM_MANAGER_H
#define WATERYSQL_SYSTEM_MANAGER_H

#include <filesystem>
#include <iostream>
#include <unordered_set>
#include <set>

#include "../errors/system_manager_error.h"
#include "../utility/type/singleton.h"
#include "../record_management/record_manager.h"
#include "../index_management/index_manager.h"

namespace watery {

class SystemManager : public Singleton<SystemManager> {

private:
    RecordManager &_record_manager = RecordManager::instance();
    IndexManager &_index_manager = IndexManager::instance();
    std::filesystem::path _base_path{std::filesystem::current_path() / DATABASE_BASE_PATH};
    std::string _current_database;
    std::set<std::string> _database_list;
    std::set<std::string> _table_list;

protected:
    SystemManager();
    
    void _scan_databases();
    void _scan_tables();
    
    template<typename Visitor>
    void _visit_field_descriptor(const std::string &table_name, const std::string &column_name, Visitor &&visit) {
        Printer::println(std::cout, "visiting table ", table_name);
        if (_table_list.count(table_name) == 0) {
            throw SystemManagerError{
                std::string{"Failed to visit table \""}.append(table_name).append("\" which does not exist.")};
        }
        auto &&record_desc = _record_manager.open_table(table_name).header.record_descriptor;
        for (auto i = 0; i < record_desc.field_count; i++) {
            auto &&field_desc = record_desc.field_descriptors[i];
            Printer::println(std::cout, "  visiting column ", field_desc.name.data());
            if (field_desc.name.data() == column_name) {
                visit(field_desc);
                return;
            }
        }
        throw SystemManagerError{
            std::string{"Failed to visit column \""}
                .append(column_name).append("\" which cannot be found in table \"")
                .append(table_name).append("\".")};
    }

public:
    void create_database(const std::string &name);
    void drop_database(const std::string &name);
    void use_database(const std::string &name);
    
    void create_table(const std::string &name, RecordDescriptor descriptor);
    void drop_table(const std::string &name);
    void create_index(const std::string &table_name, const std::string &column_name);
    void drop_index(const std::string &table_name, const std::string &column_name);
    
    const std::set<std::string> &database_list() const;
    const std::set<std::string> &table_list() const;
    
};

}

#endif  // WATERYSQL_SYSTEM_MANAGER_H
