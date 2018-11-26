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
    
    DataDescriptor data_descriptor{TypeTag::VARCHAR, 400};
    
    try {
        index_manager.create_index(name, data_descriptor);
    } catch (const std::exception &e) {
        print_error(std::cerr, e);
    }
    
    Index index{};
    
    try {
        index = index_manager.open_index(name);
        std::cout << index.header.page_count << std::endl;
        std::cout << index.header.child_count_per_node << std::endl;
        std::cout << index.header.key_length << std::endl;
        std::cout << index.header.pointer_length << std::endl;
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

