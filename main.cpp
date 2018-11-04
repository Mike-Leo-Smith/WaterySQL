#include <iostream>
#include <array>

#include "config/config.h"
#include "record_management/record_descriptor.h"
#include "data_storage/data.h"
#include "record_management/record.h"

struct A {
    static A a() {
        return A{};
    }
};

int main() {
    std::cout << "Hello, World!" << std::endl;
    
    std::array<int, 10> a{};
    std::cout << sizeof(a) << std::endl;
    
    std::cout << watery::SOME_VALUE << std::endl;
    
    std::cout << sizeof(watery::RecordDescriptor) << std::endl;
    
    auto field_descriptor = watery::FieldDescriptor{"Hello", {watery::TypeTag::INTEGER, 32}};
    auto record_descriptor = watery::RecordDescriptor{2, {field_descriptor, field_descriptor}};
    
    std::cout << record_descriptor.length() << std::endl;
    std::cout << sizeof(watery::Record) << std::endl;
    
    A x = A::a();
    
//    extern int test_filesystem();
//    return test_filesystem();
}
