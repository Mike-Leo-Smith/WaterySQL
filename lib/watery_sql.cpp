//
// Created by Mike Smith on 2019-01-01.
//

#include "watery_sql.h"
#include "../src/parsing/parser.h"
#include "../src/utility/io/error_printer.h"

void watery_sql_execute(const char *command, void (*recv)(const char *row[], unsigned long field_count)) {
    
    watery::Parser parser{command};
    try {
        parser.match()();
    } catch (const std::exception &e) {
        watery::print_error(std::cerr, e);
    }
    
}

void watery_sql_init() {
    watery::PageManager::instance();
    if (std::filesystem::exists(watery::DATABASE_BASE_PATH)) {
        std::filesystem::create_directories(watery::DATABASE_BASE_PATH);
    }
}

