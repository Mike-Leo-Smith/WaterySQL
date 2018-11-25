cmake_minimum_required(VERSION 3.12)

project(WaterySQL C CXX)

set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/opt/llvm/lib -lc++fs")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native")

set(SOURCE_FILES
    src/main.cpp
    src/page_management/filesystem_demo/bufmanager/BufferedPageManager.h
    src/page_management/filesystem_demo/bufmanager/FindReplace.h
    src/page_management/filesystem_demo/fileio/FileManager.h
    src/page_management/filesystem_demo/fileio/FileTable.h
    src/page_management/filesystem_demo/utils/compare.h
    src/page_management/filesystem_demo/utils/MyBitMap.h
    src/page_management/filesystem_demo/utils/MyHashMap.h
    src/page_management/filesystem_demo/utils/MyLinkList.h
    src/page_management/filesystem_demo/utils/pagedef.h
    src/page_management/filesystem_demo/testfilesystem.cpp
    src/config/config.h
    src/record_management/record_descriptor.h
    src/record_management/field_descriptor.h
    src/data_storage/type_tag.h
    src/data_storage/data_descriptor.h
    src/data_storage/data.h
    src/data_storage/data.cpp
    src/utility/type_constraints/non_copyable.h
    src/utility/memory_mapping/binary_stream.h
    src/utility/type_manipulators/bitset_holder_type_selector.h
    src/index_management/b_tree.cpp src/index_management/b_tree.h
    src/index_management/b_tree_node.cpp
    src/index_management/b_tree_node.h
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
    src/errors/record_manager_error.h src/index_management/b_tree_node_offset.cpp src/index_management/b_tree_node_offset.h)

add_executable(WaterySQL ${SOURCE_FILES})
