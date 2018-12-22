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
    
    std::cout << sizeof(TableHeader) << std::endl;
    
    std::ostream::sync_with_stdio(false);
    std::istream::sync_with_stdio(false);
    
    Printer::println(std::cout);
    
    Parser parser;
    while (true) {
        Printer::print(std::cout, "\n[IN]\n  ");
        std::string command;
        command.push_back(static_cast<char>(std::cin.get()));
        while (command.back() != EOF && command.back() != ';') {
            command.push_back(static_cast<char>(std::cin.get()));
        }
        Scanner exit_recognizer{command};
        auto exit_token = exit_recognizer.match_token(exit_recognizer.lookahead()).raw;
        if ((exit_token == "exit" || exit_token == "quit") && exit_recognizer.lookahead() == TokenTag::SEMICOLON) {
            Printer::println(std::cout, "Bye.");
            break;
        }
        Printer::print(std::cout, "\n[OUT]\n  ");
        try {
            parser.parse(command).match()();
        } catch (const std::exception &e) {
            print_error(std::cerr, e);
        }
    }
    
    return 0;
}
