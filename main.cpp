#include <iostream>
#include <array>

#include "config/config.h"
#include "record_management/record_descriptor.h"
#include "data_storage/data.h"
#include "record_management/record.h"
#include "record_management/record_manager.h"

int main() {
    
    using namespace watery;
    
    auto record_manager = RecordManager{};
    auto record_descriptor = RecordDescriptor{
        {"SomeThing", TypeTag::INTEGER, 4},
        {"Another", TypeTag::INTEGER, 8}
    };
    
    record_manager.create_table("test3", record_descriptor);
    if (auto &&table = record_manager.open_table("test3")) {
        std::cout << table->record_length() << std::endl;
        std::cout << table->record_count() << std::endl;
        std::cout << table->page_count() << std::endl;
        std::cout << table->slot_count_per_page() << std::endl;
        auto &&rd = table->record_descriptor();
        std::for_each_n(rd.field_descriptors.begin(), rd.field_count, [](auto &&fd) {
            std::cout << fd.name << ", " << fd.data_descriptor.size << std::endl;
        });
        record_manager.close_table(*table);
    }
    
    std::cout << sizeof(Record) << std::endl;
    std::cout << sizeof(Data) << std::endl;
    
    return 0;
    
}
