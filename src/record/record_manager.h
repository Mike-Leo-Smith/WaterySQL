//
// Created by Mike Smith on 2018/11/24.
//

#ifndef WATERYSQL_RECORD_MANAGER_H
#define WATERYSQL_RECORD_MANAGER_H

#include <string>
#include <map>
#include <memory>
#include <array>
#include "../page/page_manager.h"
#include "../data/record_descriptor.h"
#include "table.h"
#include "record_offset.h"
#include "data_page.h"
#include "../utility/memory/memory_mapper.h"

namespace watery {

class RecordManager : public Singleton<RecordManager> {

private:
    std::map<std::string, std::shared_ptr<Table>> _open_tables;

protected:
    RecordManager() = default;

public:
    ~RecordManager();
    
    void create_table(const std::string &name, const RecordDescriptor &record_descriptor);
    std::shared_ptr<Table> open_table(const std::string &name);
    void close_table(const std::string &name);
    void close_all_tables();
    void delete_table(std::string name);
    
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
