//
// Created by Mike Smith on 2018-12-26.
//

#ifndef WATERYSQL_EXECUTE_FILE_ACTOR_H
#define WATERYSQL_EXECUTE_FILE_ACTOR_H

#include <string>

#include "../parsing/parser.h"
#include "../utility/io/file_reader.h"
#include "../utility/io/error_printer.h"

namespace watery {

struct ExecuteFileActor {
    
    std::string file_name;
    
    void operator()() const {
        
        Printer::println(std::cout, "SOURCE ", file_name);
        
        std::string command{FileReader::read_all(file_name)};
        Parser parser{command};
        
        while (!parser.end()) {
            try {
                parser.match()();
            } catch (const std::exception &e) {
                print_error(std::cerr, e);
            }
        }
        
        Printer::println(std::cout, "Done.");
        
    }
    
};

}

#endif  // WATERYSQL_EXECUTE_FILE_ACTOR_H
