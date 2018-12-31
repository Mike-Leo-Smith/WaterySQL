//
// Created by Mike Smith on 2019-01-01.
//

#include "watery_sql.h"
#include "../src/parsing/parser.h"
#include "../src/utility/io/error_printer.h"

namespace watery {

void execute_sql(const std::string &command, const std::function<void(const std::vector<std::string> &)> &recv) {
    
    Parser parser{command};
    try {
        parser.match()();
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
}

}
