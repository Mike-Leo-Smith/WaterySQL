//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_FILE_MANAGER_H
#define WATERYSQL_FILE_MANAGER_H

#include <string>
#include <fstream>

namespace watery {

class FileManager {
public:

private:

public:
    static FileManager &instance();
    
    void create_file(std::string file_name);
    std::fstream open_file(std::string file_name);
    
};

}

#endif  // WATERYSQL_FILE_MANAGER_H
