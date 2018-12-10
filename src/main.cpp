#include <iostream>
#include <string_view>

#include "parsing/scanner.h"
#include "system_management/system_manager.h"
#include "index_management/index_manager.h"

#include "utility/io_helpers/error_printer.h"
#include "parsing/parser.h"

#include "data_storage/data_comparator.h"

int main() {
    
    using namespace watery;
    
    constexpr auto query = "select (*.app_le) from PERSON\n where age<>'apple';\n";
    Scanner scanner{query};
    
    auto t = scanner.lookahead();
    while (t != TokenTag::END) {
        auto token = scanner.match_token(t);
        std::cout << token.raw << " @ line #" << token.offset.line << ", col #" << token.offset.column << std::endl;
        t = scanner.lookahead();
    }
    
    auto &&system_manager = SystemManager::instance();
    
    Parser parser;
    
    parser.parse("drop database test;")();
    parser.parse("create database test;")();
    parser.parse("use database test;")();
    
    auto &&index_manager = IndexManager::instance();
    
    std::string name{"test4"};
    
    DataDescriptor data_descriptor{TypeTag::INTEGER, true, 10};
    FieldDescriptor field_desc{"test", data_descriptor};

    try {
        index_manager.create_index(name, field_desc);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    parser.parse("Show Databases;")();
    
    return 0;
    
}
