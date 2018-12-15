//
// Created by Mike Smith on 2018/11/25.
//


#include <iostream>
#include <array>
#include <random>
#include <algorithm>
#include <chrono>

#include "../src/config/config.h"
#include "../src/data_storage/record_descriptor.h"
#include "../src/data_storage/data.h"
#include "../src/data_storage/char.h"
#include "../src/errors/page_manager_error.h"
#include "../src/page_management/page_manager.h"

#include "../src/utility/io/error_printer.h"
#include "../src/utility/timing/elapsed_time.h"
#include "../src/record_management/record_manager.h"

#include "../src/index_management/index_manager.h"

int main() {
    
    using namespace watery;
    
    auto &&index_manager = IndexManager::instance();
    
    std::string name{"test4"};
    
    try {
        index_manager.delete_index(name);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    DataDescriptor data_descriptor{TypeTag::INTEGER, 10};
    FieldDescriptor field_desc{"test", data_descriptor, FieldConstraint{}};
    
    try {
        index_manager.create_index(name, field_desc);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    Index index{};
    
    try {
        index = index_manager.open_index(name);
        
        index_manager.insert_index_entry(index, reinterpret_cast<const Byte *>("hello, world"), {1, 2});
        index_manager.delete_index_entry(index, reinterpret_cast<const Byte *>("hello, world"), {1, 2});
        
        constexpr auto count = 5'000'000;
        std::default_random_engine random{std::random_device{}()};
        
        std::vector<int32_t> data_set;
        data_set.reserve(count);
        for (auto i = 0; i < count; i++) {
            data_set.emplace_back(i);
        }
        
//        std::vector<std::string> data_set;
//        data_set.reserve(count);
//        for (auto i = 0; i < count; i++) {
//            const auto *padding = "0000000";
//            auto n = std::to_string(i);
//            auto s = std::string{&padding[n.size()]}.append(n);
//            for (auto j = 0; j < 20; j++) {
//                std::uniform_int_distribution<char> dist{0x20, 0x7e};
//                s += dist(random);
//            }
//            data_set.emplace_back(s);
//        }

        RecordOffset rid{1, 2};
        
        std::cout << "-------- testing insertion ---------" << std::endl;
        {
            std::shuffle(data_set.begin(), data_set.end(), random);
            std::cout << "elapsed time: " << timed_run([&] {
                for (auto &&entry: data_set) {
                    index_manager.insert_index_entry(index, reinterpret_cast<const Byte *>(&entry), rid);
                }
            }).first << "ms" << std::endl;
        }
        
        std::cout << "------- testing search --------" << std::endl;
        {
            std::shuffle(data_set.begin(), data_set.end(), random);
            std::cout << "elapsed time: " << timed_run([&] {
                for (auto &&entry: data_set) {
                    index_manager.search_index_entry(index, reinterpret_cast<const Byte *>(&entry));
                }
            }).first << "ms" << std::endl;
        }
        
        std::cout << "------- testing deletion --------" << std::endl;
        {
            std::shuffle(data_set.begin(), data_set.end(), random);
            std::cout << "elapsed time: " << timed_run([&] {
                for (auto &&entry: data_set) {
                    index_manager.delete_index_entry(index, reinterpret_cast<const Byte *>(&entry), rid);
                }
            }).first << "ms" << std::endl;
        }
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    index_manager.close_index(name);
    
    return 0;
    
}

