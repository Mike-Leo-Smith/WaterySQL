#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <variant>

#include "parsing/scanner.h"
#include "system/system_manager.h"
#include "index/index_manager.h"

#include "utility/io/error_printer.h"
#include "parsing/parser.h"

#include "data/data_comparator.h"
#include "utility/io/file_reader.h"
#include "utility/time/elapsed_time.h"
#include "utility/memory/value_decoder.h"

int main() {
    
    using namespace watery;
    
    std::ostream::sync_with_stdio(false);
    std::istream::sync_with_stdio(false);
    
    Printer::println(std::cout);
    
    Parser parser;
    while (true) {
        Printer::print(std::cout, "\n[IN]\n");
        std::string command;
        command.push_back(static_cast<char>(std::cin.get()));
        while (command.back() != EOF && command.back() != ';') {
            command.push_back(static_cast<char>(std::cin.get()));
        }
        Printer::print(std::cout, "\n[OUT]\n");
        try {
            parser.parse(command).match()();
        } catch (const std::exception &e) {
            print_error(std::cerr, e);
        }
    }
    
    return 0;
}
