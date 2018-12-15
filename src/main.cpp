#include <iostream>
#include <fstream>
#include <string>
#include <string_view>

#include "parsing/scanner.h"
#include "system_management/system_manager.h"
#include "index_management/index_manager.h"

#include "utility/io_helpers/error_printer.h"
#include "parsing/parser.h"

#include "data_storage/data_comparator.h"
#include "utility/io_helpers/reader.h"
#include "utility/timing/elapsed_time.h"

int main() {
    
    using namespace watery;
    
    char a[3] {'A', '\0', '\0'};
    std::cout << std::string_view{a, 3} << std::endl;
    
    std::ostream::sync_with_stdio(false);
    
    constexpr auto query = "select (*.app_le) from PERSON\n where age<>'apple';\n";
    Scanner scanner{query};
    
    while (!scanner.end()) {
        auto token = scanner.match_token(scanner.lookahead());
        std::cout << token.raw << " @ line #" << token.offset.line << ", col #" << token.offset.column << std::endl;
    }
    
    auto &&system_manager = SystemManager::instance();
    
    Parser parser;
    
    parser.parse("drop database orderDB;").match()();
    
    parser.parse(FileReader::read("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/create.sql"));
    while (!parser.end()) {
        parser.match()();
    }
    
    parser.parse(
        "create table test("
        "  col1 int(5), "
        "  col2 int nul, "
        "  col3 float, "
        "  col4 date(20) not null, "
        "  foreign key(col3) references another(some), "
        "  unique(col2),"
        "  primary key(col3));"
        "create table test2("
        "  col1 int not null);");
    while (!parser.end()) {
        try {
            parser.match()();
        } catch (const std::exception &e) {
            print_error(std::cerr, e);
            parser.skip();
        }
    }
    
    parser.parse("drop database test;").match()();
    parser.parse("create database test;").match()();
    parser.parse("use test;").match()();
    
    parser.parse("insert into test values (123, 456, 'apple'), (null);").match()();
    
    auto &&index_manager = IndexManager::instance();
    
    std::string name{"test4"};
    
    DataDescriptor data_descriptor{TypeTag::INTEGER, 10};
    FieldDescriptor field_desc{"test", data_descriptor, FieldConstraint{}};
    
    try {
        index_manager.create_index(name, field_desc);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    parser.parse("Show Databases;").match()();
    
    std::cout << sizeof(std::bitset<1024>) << std::endl;
    std::cout << "elapsed time: " << timed_run([&parser]() {
        parser.parse(FileReader::read("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/food.sql")).match()();
        parser.parse(FileReader::read("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/orders.sql")).match()();
        parser.parse(FileReader::read("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/customer.sql")).match()();
        parser.parse(FileReader::read("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/restaurant.sql")).match()();
    }).first << "ms" << std::endl;
    
    return 0;
    
}
