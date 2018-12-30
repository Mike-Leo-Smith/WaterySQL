//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_FILE_READER_H
#define WATERYSQL_FILE_READER_H

#include <string>
#include <string_view>
#include <filesystem>
#include <fstream>

#include "../type/non_constructible.h"

namespace watery {

struct FileReader : NonConstructible {
    
    static std::string_view read_all(const std::string &file_name) {
        thread_local static std::string buffer;
        auto size = std::filesystem::file_size(file_name);
        buffer.resize(size);
        std::ifstream{file_name}.read(buffer.data(), size);
        return buffer;
    }
    
};

}

#endif  // WATERYSQL_FILE_READER_H
