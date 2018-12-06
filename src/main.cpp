#include <iostream>
#include <string_view>

#include "command_parsing/scanner.h"

int main() {
    
    using namespace watery;
    
    constexpr auto query = "select (*.apple) from PERSON\n where age<>123.456.7;\n";
    Scanner scanner{query};
    
    auto t = scanner.lookahead();
    while (t != TokenTag::END) {
        auto token = scanner.match_token(t);
        std::cout << token.raw << " @ line #" << token.offset.line << ", col #" << token.offset.column << std::endl;
        t = scanner.lookahead();
    }
    
    return 0;
    
}
