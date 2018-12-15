cmake_minimum_required(VERSION 3.12)

project(WaterySQL C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(COMPILE_WITH_CLANG 1)

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
    src/page_management/filesystem_demo/BufferedPageManager.h
    src/page_management/filesystem_demo/FindReplace.h
    src/page_management/filesystem_demo/FileManager.h
    src/page_management/filesystem_demo/MyHashMap.h
    src/page_management/filesystem_demo/MyLinkList.h
    src/config/config.h
    src/data_storage/record_descriptor.h
    src/data_storage/field_descriptor.h
    src/data_storage/type_tag.h
    src/data_storage/data_descriptor.h
    src/utility/type/non_copyable.h
    src/index_management/index_manager.cpp
    src/index_management/index_manager.h
    src/page_management/page_manager.cpp
    src/page_management/page_manager.h
    src/utility/type/singleton.h
    src/utility/type/non_movable.h
    src/errors/page_manager_error.h
    src/page_management/buffered_page.h
    src/utility/io/error_printer.h
    src/utility/io/printer.h
    src/errors/error.h
    src/utility/type/non_trivial_constructible.h
    src/record_management/record_offset.h
    src/record_management/record_manager.cpp
    src/record_management/record_manager.h
    src/record_management/table.h
    src/record_management/data_page_header.h
    src/utility/memory/memory_mapper.h
    src/page_management/buffer_offset.h
    src/record_management/table_header.h
    src/record_management/data_page.h
    src/errors/record_manager_error.h
    src/index_management/index_node.h
    src/index_management/index.h
    src/index_management/index_header.h
    src/index_management/index_node_header.h
    src/errors/index_manager_error.h
    src/index_management/index_entry_offset.h
    src/errors/data_error.h
    src/index_management/index_node_link.h
    src/utility/mathematics/sgn.h
    src/utility/time/elapsed_time.h
    src/system_management/system_manager.cpp
    src/system_management/system_manager.h
    src/parsing/token.h
    src/parsing/token_tag.h
    src/errors/scanner_error.h
    src/parsing/scanner.cpp
    src/parsing/scanner.h
    src/parsing/parser.cpp
    src/parsing/parser.h
    src/parsing/token_offset.h
    src/errors/system_manager_error.h
    src/execution/show_databases_actor.h
    src/errors/parser_error.h
    src/execution/show_tables_actor.h
    src/execution/actor.h
    src/execution/create_database_actor.h
    src/execution/use_database_actor.h
    src/execution/drop_database_actor.h
    src/data_storage/field_constraint.h
    src/data_storage/data_comparator.h
    src/data_storage/index_key_comparator.h src/execution/create_table_actor.h src/execution/create_index_actor.h src/execution/drop_index_actor.h src/execution/drop_table_actor.h src/execution/describe_table_actor.h src/parsing/token_tag_helper.h src/parsing/token_tag_helper.cpp src/execution/insert_record_actor.h src/utility/io/reader.h src/utility/memory/string_view_copier.h src/data_storage/data_view.h src/utility/memory/value_decoder.h src/errors/value_decoder_error.h src/utility/time/date_validator.h src/execution/column_predicate.h src/execution/delete_record_actor.h src/execution/column_predicate_operator.h)

add_executable(WaterySQL ${SOURCE_FILES} src/main.cpp)
add_executable(WaterySQL-RecordManagerTest ${SOURCE_FILES} test/record_manager_test.cpp)
add_executable(WaterySQL-IndexManagerTest ${SOURCE_FILES} test/index_manager_test.cpp)
