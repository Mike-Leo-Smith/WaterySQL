//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_RECORD_MANAGER_H
#define WATERYSQL_RECORD_MANAGER_H

#include <string>

namespace watery {

class RecordManager {

public:
    void create_table(const std::string &name);
    void open_table(const std::string &name);
    
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
