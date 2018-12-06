cmake_minimum_required(VERSION 3.12)

project(WaterySQL C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
set(CMAKE_EXE_LINKER_FLAGS " ${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/opt/llvm/lib -lc++fs")

#set(CMAKE_C_COMPILER gcc-8)
#set(CMAKE_CXX_COMPILER g++-8)
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lstdc++fs")

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -mtune=native -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -mtune=native -fomit-frame-pointer")

set(SOURCE_FILES
    src/page_management/filesystem_demo/BufferedPageManager.h
    src/page_management/filesystem_demo/FindReplace.h
    src/page_management/filesystem_demo/FileManager.h
    src/page_management/filesystem_demo/MyHashMap.h
    src/page_management/filesystem_demo/MyLinkList.h
    src/config/config.h
    src/record_management/record_descriptor.h
    src/record_management/field_descriptor.h
    src/data_storage/type_tag.h
    src/data_storage/data_descriptor.h
    src/data_storage/data.h
    src/utility/type_constraints/non_copyable.h
    src/utility/memory_mapping/binary_stream.h
    src/utility/type_manipulators/bitset_holder_type_selector.h
    src/index_management/index_manager.cpp
    src/index_management/index_manager.h
    src/page_management/page_manager.cpp
    src/page_management/page_manager.h
    src/utility/type_constraints/singleton.h
    src/utility/type_constraints/non_movable.h
    src/errors/page_manager_error.h
    src/page_management/buffered_page.h
    src/utility/io_helpers/error_printer.h
    src/utility/io_helpers/printer.h
    src/errors/error.h
    src/utility/type_constraints/non_trivial_constructible.h
    src/record_management/record_offset.h
    src/record_management/record_manager.cpp
    src/record_management/record_manager.h
    src/record_management/table.h
    src/record_management/data_page_header.h
    src/utility/memory_mapping/memory_mapper.h
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
    src/data_storage/integer.h
    src/data_storage/data.cpp
    src/data_storage/float.h
    src/data_storage/varchar.h
    src/index_management/index_node_link.h
    src/utility/mathematics/sgn.h
    src/utility/timing/elapsed_time.h src/system_management/system_manager.cpp src/system_management/system_manager.h src/command_parsing/token.h src/command_parsing/token_tag.h src/errors/scanner_error.h)

add_executable(WaterySQL ${SOURCE_FILES} src/main.cpp src/command_parsing/scanner.cpp src/command_parsing/scanner.h src/command_parsing/parser.cpp src/command_parsing/parser.h src/command_parsing/token_offset.h)
add_executable(WaterySQL-RecordManagerTest ${SOURCE_FILES} test/record_manager_test.cpp)
add_executable(WaterySQL-IndexManagerTest ${SOURCE_FILES} test/index_manager_test.cpp)
