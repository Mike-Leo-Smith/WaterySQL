#include <iostream>
#include <string_view>

#include "parsing/scanner.h"
#include "system_management/system_manager.h"
#include "index_management/index_manager.h"

#include "utility/io_helpers/error_printer.h"

int main() {
    
    using namespace watery;
    
    constexpr auto query = "select (*.apple) from PERSON\n where age<>123.456.7;\n";
    Scanner scanner{query};
    
    auto t = scanner.lookahead();
    while (t != TokenTag::END) {
        auto token = scanner.match_token(t);
        std::cout << token.raw << " @ line #" << token.offset.line << ", col #" << token.offset.column << std::endl;
        t = scanner.lookahead();
    }
    
    auto &&system_manager = SystemManager::instance();
    
    system_manager.delete_database("test");
    system_manager.create_database("test");
    system_manager.use_database("test");
    
    auto &&index_manager = IndexManager::instance();
    
    std::string name{"test4"};
    
    try {
        index_manager.delete_index(name);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    DataDescriptor data_descriptor{TypeTag::INTEGER, 10};
    
    try {
        index_manager.create_index(name, data_descriptor);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    for (auto &&table : system_manager.all_databases()) {
        std::cout << table << std::endl;
    }
    
    return 0;
    
}
