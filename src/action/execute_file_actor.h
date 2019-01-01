//
// Created by Mike Smith on 2018-12-26.
//

#ifndef WATERYSQL_EXECUTE_FILE_ACTOR_H
#define WATERYSQL_EXECUTE_FILE_ACTOR_H

#include <string>
#include <fstream>

#include "../parsing/parser.h"
#include "../utility/io/file_reader.h"
#include "../utility/io/error_printer.h"

namespace watery {

struct ExecuteFileActor {
    
    std::string file_name;
    
    void operator()() const {
    
        {
            std::ofstream f{RESULT_FILE_NAME, std::ios::app};
            Printer::println(f, "SOURCE ", file_name);
        }
        
        std::string command{FileReader::read_all(file_name)};
        Parser parser{command};
        
        auto ms = timed_run([&] {
            while (!parser.end()) {
                parser.match()();
            }
        }).first;
        
        std::ofstream f{RESULT_FILE_NAME, std::ios::app};
        Printer::println(f, "Done in ", ms, "ms.<br/>");
        
    }
    
};

}

#endif  // WATERYSQL_EXECUTE_FILE_ACTOR_H
