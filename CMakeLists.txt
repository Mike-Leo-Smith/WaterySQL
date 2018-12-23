cmake_minimum_required(VERSION 3.12)

project(WaterySQL C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(COMPILE_WITH_CLANG 0)

if (COMPILE_WITH_CLANG)
    set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
    set(CMAKE_EXE_LINKER_FLAGS " ${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/opt/llvm/lib -lc++fs")
else ()
    set(CMAKE_C_COMPILER gcc-8)
    set(CMAKE_CXX_COMPILER g++-8)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")
endif ()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -mtune=native -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -mtune=native -fomit-frame-pointer")

set(SOURCE_FILES
    src/config/config.h
    src/data/record_descriptor.h
    src/data/field_descriptor.h
    src/data/type_tag.h
    src/data/data_descriptor.h
    src/utility/type/non_copyable.h
    src/index/index_manager.cpp
    src/index/index_manager.h
    src/page/page_manager.cpp
    src/page/page_manager.h
    src/utility/type/singleton.h
    src/utility/type/non_movable.h
    src/error/page_manager_error.h
    src/utility/io/error_printer.h
    src/utility/io/printer.h
    src/error/error.h
    src/utility/type/non_trivial_constructible.h
    src/record/record_offset.h
    src/record/record_manager.cpp
    src/record/record_manager.h
    src/record/table.h
    src/record/data_page_header.h
    src/utility/memory/memory_mapper.h
    src/record/table_header.h
    src/record/data_page.h
    src/error/record_manager_error.h
    src/index/index_node.h
    src/index/index.h
    src/index/index_header.h
    src/index/index_node_header.h
    src/index/index_entry_offset.h
    src/index/index_node_link.h
    src/error/index_manager_error.h
    src/error/data_error.h
    src/utility/mathematics/sgn.h
    src/utility/time/elapsed_time.h
    src/system/system_manager.cpp
    src/system/system_manager.h
    src/parsing/token.h
    src/parsing/token_tag.h
    src/error/scanner_error.h
    src/parsing/scanner.cpp
    src/parsing/scanner.h
    src/parsing/parser.cpp
    src/parsing/parser.h
    src/parsing/token_offset.h
    src/error/system_manager_error.h
    src/action/show_databases_actor.h
    src/error/parser_error.h
    src/action/show_tables_actor.h
    src/action/actor.h
    src/action/create_database_actor.h
    src/action/use_database_actor.h
    src/action/drop_database_actor.h
    src/data/field_constraint.h
    src/data/data_comparator.h
    src/data/index_key_comparator.h
    src/action/create_table_actor.h
    src/action/create_index_actor.h
    src/action/drop_index_actor.h
    src/action/drop_table_actor.h
    src/action/describe_table_actor.h
    src/parsing/token_tag_helper.h
    src/parsing/token_tag_helper.cpp
    src/action/insert_record_actor.h
    src/utility/io/reader.h
    src/utility/memory/string_view_copier.h
    src/data/data_view.h
    src/utility/memory/value_decoder.h
    src/error/value_decoder_error.h
    src/utility/time/date_validator.h
    src/action/column_predicate.h
    src/action/delete_record_actor.h
    src/action/column_predicate_operator.h
    src/action/column_predicate_helper.h
    src/action/update_record_actor.h
    src/action/select_record_actor.h
    src/utility/memory/identifier_comparison.h
    src/page/page_handle.h src/action/exit_actor.h src/utility/io/record_descriptor_printer.h)

add_executable(WaterySQL ${SOURCE_FILES} src/main.cpp)
add_executable(WaterySQL-RecordManagerTest ${SOURCE_FILES} test/record_manager_test.cpp)
add_executable(WaterySQL-IndexManagerTest ${SOURCE_FILES} test/index_manager_test.cpp)
add_executable(WaterySQL-CommandParsingTest ${SOURCE_FILES} test/command_parsing_test.cpp)
