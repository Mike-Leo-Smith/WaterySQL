//
// Created by Mike Smith on 2018/11/25.
//


#include <chrono>
#include <iostream>
#include <array>
#include <algorithm>
#include <functional>

#include "../src/config/config.h"
#include "../src/data_storage/record_descriptor.h"
#include "../src/errors/page_manager_error.h"
#include "../src/page_management/page_manager.h"

#include "../src/utility/io/error_printer.h"
#include "../src/record_management/record_manager.h"

int main() {
    
    using namespace watery;
    
    auto &&record_manager = RecordManager::instance();
    
    auto name = "test_fs";
    try {
        record_manager.delete_table(name);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    FieldConstraint c{FieldConstraint::UNIQUE_BIT_MASK};
    auto record_descriptor = RecordDescriptor{
        FieldDescriptor{"SomeThing", DataDescriptor{TypeTag::INTEGER, 4}, c},
        FieldDescriptor{"Another", DataDescriptor{TypeTag::INTEGER, 8}, c},
        FieldDescriptor{"Another", DataDescriptor{TypeTag::INTEGER, 8}, c},
        FieldDescriptor{"Another", DataDescriptor{TypeTag::INTEGER, 8}, c},
        FieldDescriptor{"Another", DataDescriptor{TypeTag::INTEGER, 8}, c},
    };
    
    record_manager.create_table(name, record_descriptor);
    
    {
        auto table = record_manager.open_table(name);
        std::cout << table.lock()->header.record_length << std::endl;
        std::cout << table.lock()->header.record_count << std::endl;
        std::cout << table.lock()->header.page_count << std::endl;
        auto &&rd = table.lock()->header.record_descriptor;
        std::for_each(rd.field_descriptors.begin(), rd.field_descriptors.begin() + rd.field_count, [](auto &&fd) {
            std::cout << fd.name.data() << ", " << fd.data_descriptor.length() << std::endl;
        });
        
        std::cout << "------- inserting --------" << std::endl;
        record_manager.insert_record(table, "Hello, World!!! I am happy!!!");
        record_manager.insert_record(table, "Hello, Luisa!!! I am happy!!!");
        auto r = record_manager.insert_record(table, "Hello, Mike!!! I am happy!!!");
        record_manager.insert_record(table, "Hello, John!!! I am happy!!!");
    }
    
    try {
        record_manager.close_table(name);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    {
        std::cout << "------- retrieving -------" << std::endl;
        auto table = record_manager.open_table(name);
        for (auto slot = 0; slot < 4; slot++) {
            auto buffer = record_manager.get_record(table, {1, slot});
            for (auto i = 0; i < 15; i++) {
                std::cout << buffer[i];
            }
            std::cout << std::endl;
        }
        
        std::cout << "------- speed test -------" << std::endl;
        auto start = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < 10'000'000; i++) {
            record_manager.insert_record(table, "Hello, World!!! I am happy!!!");
        }
        auto stop = std::chrono::high_resolution_clock::now();
        using namespace std::chrono_literals;
        std::cout << "time: " << (stop - start) / 1ms << "ms" << std::endl;
        
        try {
            record_manager.close_table(name);
        } catch (const std::exception &e) {
            print_error(std::cerr, e);
        }
    }
    
    std::cout << sizeof(DataPageHeader) << std::endl;
    
    return 0;
    
}

