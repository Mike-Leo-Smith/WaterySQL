#include <iostream>
#include <fstream>
#include <string>
#include <string_view>
#include <variant>

#include "../src/parsing/scanner.h"
#include "../src/system_management/system_manager.h"
#include "../src/index_management/index_manager.h"

#include "../src/utility/io/error_printer.h"
#include "../src/parsing/parser.h"

#include "../src/data_storage/data_comparator.h"
#include "../src/utility/io/reader.h"
#include "../src/utility/time/elapsed_time.h"
#include "../src/utility/memory/value_decoder.h"

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
    parser.parse(FileReader::read_all("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/create.sql"));
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
    
    parser.parse("create database test;").match()();
    parser.parse("use test;").match()();
    parser.parse("insert into test values (123, 456, 'apple'), (null);").match()();
    parser.parse("drop database test;").match()();
    
    auto &&index_manager = IndexManager::instance();
    
    std::string name{"test4"};
    
    DataDescriptor data_descriptor{TypeTag::INTEGER, 10};
    
    try {
        index_manager.create_index(name, data_descriptor, true);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    parser.parse("Show Databases;").match()();
    
    std::cout << sizeof(std::bitset<1024>) << std::endl;
    std::cout << "elapsed time: " << timed_run([&parser]() {
        parser.parse(FileReader::read_all("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/food.sql")).match()();
        parser.parse(FileReader::read_all("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/orders.sql")).match()();
        parser.parse(FileReader::read_all("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/customer.sql")).match()();
        parser.parse(FileReader::read_all("/Users/mike/Desktop/数据库/WaterySQL/res/dataset_small/restaurant.sql")).match()();
    }).first << "ms" << std::endl;
    
    std::cout << sizeof(std::variant<char, int, short, std::array<int, 5>>) << std::endl;
    
    std::variant<int, float> x{11};
    std::cout << std::holds_alternative<float>(x) << std::endl;
    
    parser.parse("delete from `some_table` where `some_table`.`x` = 0 and y < 5 and z is not null;").match()();
    parser.parse("delete from `some_table`;").match()();
    
    parser.parse("update test_tab set aaa = 0, bb = NULL where aaa = 1 and b = 2;").match()();
    parser.parse("select app.le, badGirl from boy, girl where a = 1 and b = 0;").match()();
    
    auto date = static_cast<uint32_t>(ValueDecoder::decode_date("'1998-08-10'"));
    std::cout << (date >> 16) << "-" << ((date >> 24) & 0xff) << "-" << (date & 0xff) << std::endl;
    
    return 0;
    
}
