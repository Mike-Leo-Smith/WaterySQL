//
// Created by Mike Smith on 2018/11/25.
//


#include <iostream>
#include <array>

#include "../src/config/config.h"
#include "../src/record_management/record_descriptor.h"
#include "../src/data_storage/data.h"
#include "../src/data_storage/varchar.h"
#include "../src/errors/page_manager_error.h"
#include "../src/page_management/page_manager.h"

#include "../src/utility/io_helpers/error_printer.h"
#include "../src/record_management/record_manager.h"

#include "../src/index_management/index_manager.h"

int main() {
    
    using namespace watery;
    
    auto &&index_manager = IndexManager::instance();
    
    std::string name{"test3.idx"};
    
    try {
        index_manager.delete_index(name);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    DataDescriptor data_descriptor{TypeTag::VARCHAR, 10};
    
    try {
        index_manager.create_index(name, data_descriptor);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    Index index{};
    
    try {
        index = index_manager.open_index(name);
        std::cout << index.header.page_count << std::endl;
        std::cout << index.header.key_count_per_node << std::endl;
        std::cout << index.header.key_length << std::endl;
        std::cout << index.header.pointer_length << std::endl;
        
        auto &&decode_data = [data_descriptor](std::string_view s) {
            return Data::decode(data_descriptor, reinterpret_cast<const Byte *>(s.data()));
        };
        index_manager.insert_index_entry(index, decode_data("hello5566, my dear!"), RecordOffset{0xcc, 0xcc});
        index_manager.insert_index_entry(index, decode_data("hello, my dear!"), RecordOffset{0xcc, 0xcc});
        index_manager.insert_index_entry(index, decode_data("hello3344, my dear!"), RecordOffset{0xcc, 0xcc});
        index_manager.insert_index_entry(index, decode_data("hello2233, my dear!"), RecordOffset{0xcc, 0xcc});
        
        auto start = std::chrono::high_resolution_clock::now();
        for (auto i = 0; i < 500000; i++) {
            auto k = decode_data(std::to_string(rand()).append("helloooooo!!!"));
            auto rid = RecordOffset{};
            index_manager.insert_index_entry(index, k, rid);
        }
        auto stop = std::chrono::high_resolution_clock::now();
        using namespace std::chrono_literals;
        std::cout << "elapsed time: " << (stop - start) / 1ms << "ms" << std::endl;
        
        index_manager.delete_index_entry(index, decode_data("hello3344, my dear!"), RecordOffset{0xcc, 0xcc});
        
        std::cout << "------- testing search --------" << std::endl;
        auto entry = index_manager.search_index_entry(index, decode_data("hello3344, my dear!"));
        std::cout << entry.page_offset << std::endl;
        std::cout << entry.child_offset << std::endl;
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    try {
        index_manager.close_index(index);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    return 0;
    
}

