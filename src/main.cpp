#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <variant>

#include "parsing/scanner.h"
#include "system_management/system_manager.h"
#include "index_management/index_manager.h"

#include "utility/io/error_printer.h"
#include "parsing/parser.h"

#include "data_storage/data_comparator.h"
#include "utility/io/reader.h"
#include "utility/time/elapsed_time.h"
#include "utility/memory/value_decoder.h"

int main() {
    
    using namespace watery;
    
    std::ostream::sync_with_stdio(false);
    std::istream::sync_with_stdio(false);
    
    Printer::println(std::cout);
    
    Parser parser;
    while (true) {
        Printer::print(std::cout, "[IN]\n  ");
        std::string command;
        command.push_back(static_cast<char>(std::cin.get()));
        while (command.back() != EOF && command.back() != ';') {
            command.push_back(static_cast<char>(std::cin.get()));
        }
        if (command.find("exit;") != std::string::npos) {
            Printer::println(std::cout, "Bye.");
            break;
        }
        Printer::print(std::cout, "[OUT]\n  ");
        parser.parse(command).match()();
    }
    
    return 0;
}
