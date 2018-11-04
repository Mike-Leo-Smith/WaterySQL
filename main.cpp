#include <iostream>

int main() {
    std::cout << "Hello, World!" << std::endl;
    
    extern int test_filesystem();
    return test_filesystem();
}
