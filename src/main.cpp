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

int main() {
    
    using namespace watery;
    
    constexpr auto query = "select (*.app_le) from PERSON\n where age<>'apple';\n";
    Scanner scanner{query};
    
    while (!scanner.end()) {
        auto token = scanner.match_token(scanner.lookahead());
        std::cout << token.raw << " @ line #" << token.offset.line << ", col #" << token.offset.column << std::endl;
    }
    
    auto &&system_manager = SystemManager::instance();
    
    Parser parser;
    
    parser.parse("drop database orderDB;").next()();
    
    std::ifstream fin{"/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/create.sql"};
    std::string s;
    while (!fin.eof()) {
        s.push_back(static_cast<char>(fin.get()));
    }
    
    parser.parse(s);
    while (!parser.end()) {
        parser.next()();
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
            parser.next()();
        } catch (const std::exception &e) {
            print_error(std::cerr, e);
            parser.skip();
        }
    }
    
    parser.parse("drop database test;").next()();
    parser.parse("create database test;").next()();
    parser.parse("use test;").next()();
    
    auto &&index_manager = IndexManager::instance();
    
    std::string name{"test4"};
    
    DataDescriptor data_descriptor{TypeTag::INTEGER, 10};
    FieldDescriptor field_desc{"test", data_descriptor, FieldConstraint{}};
    
    try {
        index_manager.create_index(name, field_desc);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    parser.parse("Show Databases;").next()();
    
    return 0;
    
}
