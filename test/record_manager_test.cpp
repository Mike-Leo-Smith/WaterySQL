//
// Created by Mike Smith on 2018/11/25.
//


#include <iostream>
#include <array>

#include "../src/config/config.h"
#include "../src/record_management/record_descriptor.h"
#include "../src/data_storage/data.h"
#include "../src/errors/page_manager_error.h"
#include "../src/page_management/page_manager.h"

#include "../src/utility/io_helpers/error_printer.h"
#include "../src/record_management/record_manager.h"

int main() {
    
    using namespace watery;
    
    auto &&record_manager = RecordManager::instance();
    
    auto name = "test3.tab";
    try {
        record_manager.delete_table(name);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    auto record_descriptor = RecordDescriptor{
        {"SomeThing", TypeTag::INTEGER, 4},
        {"Another",   TypeTag::INTEGER, 8},
        {"Another",   TypeTag::INTEGER, 8},
        {"Another",   TypeTag::INTEGER, 8},
        {"Another",   TypeTag::INTEGER, 8},
    };
    
    try {
        record_manager.create_table(name, record_descriptor);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    Table table{};
    
    try {
        table = record_manager.open_table(name);
        std::cout << table.header.record_length << std::endl;
        std::cout << table.header.record_count << std::endl;
        std::cout << table.header.page_count << std::endl;
        auto &&rd = table.header.record_descriptor;
        std::for_each_n(rd.field_descriptors.begin(), rd.field_count, [](auto &&fd) {
            std::cout << fd.name << ", " << fd.data_descriptor.length << std::endl;
        });
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    try {
        
        std::cout << "------- inserting --------" << std::endl;
        record_manager.insert_record(table, reinterpret_cast<const Byte *>("Hello, World!!! I am happy!!!"));
        record_manager.insert_record(table, reinterpret_cast<const Byte *>("Hello, Luisa!!! I am happy!!!"));
        auto r = record_manager.insert_record(table, reinterpret_cast<const Byte *>("Hello, Mike!!! I am happy!!!"));
        record_manager.insert_record(table, reinterpret_cast<const Byte *>("Hello, John!!! I am happy!!!"));
        
        record_manager.delete_record(table, r);
        
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    try {
        record_manager.close_table(table);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    std::cout << "------- retrieving -------" << std::endl;
    record_manager.open_table(name);
    for (auto slot = 0; slot < 4; slot++) {
        try {
            auto buffer = reinterpret_cast<const char *>(record_manager.get_record(table, {1, slot}));
            for (auto i = 0; i < 15; i++) {
                std::cout << buffer[i];
            }
            std::cout << std::endl;
        } catch (const std::exception &e) {
            print_error(std::cerr, e);
        }
    }
    
    std::cout << "------- speed test -------" << std::endl;
    auto start = std::chrono::high_resolution_clock::now();
    for (auto i = 0; i < 1000000; i++) {
        record_manager.insert_record(table, reinterpret_cast<const Byte *>("Hello, World!!! I am happy!!!"));
    }
    auto stop = std::chrono::high_resolution_clock::now();
    using namespace std::chrono_literals;
    std::cout << "time: " << (stop - start) / 1ms << "ms" << std::endl;
    
    try {
        record_manager.close_table(table);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    std::cout << sizeof(Data) << std::endl;
    std::cout << sizeof(DataPageHeader) << std::endl;
    
    return 0;
    
}

