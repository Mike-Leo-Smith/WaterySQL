cmake_minimum_required(VERSION 3.12)

project(WaterySQL C CXX)

set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -L/usr/local/opt/llvm/lib -lc++fs")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native")

set(SOURCE_FILES
    src/main.cpp
    src/filesystem_demo/bufmanager/BufferedPageManager.h
    src/filesystem_demo/bufmanager/FindReplace.h
    src/filesystem_demo/fileio/FileManager.h
    src/filesystem_demo/fileio/FileTable.h
    src/filesystem_demo/utils/compare.h
    src/filesystem_demo/utils/MyBitMap.h
    src/filesystem_demo/utils/MyHashMap.h
    src/filesystem_demo/utils/MyLinkList.h
    src/filesystem_demo/utils/pagedef.h
    src/filesystem_demo/testfilesystem.cpp
    src/record_management/record_manager.cpp
    src/record_management/record_manager.h
    src/config/config.h
    src/record_management/record_descriptor.h
    src/record_management/field_descriptor.h
    src/data_storage/type_tag.h
    src/data_storage/data_descriptor.h
    src/data_storage/data.h
    src/data_storage/data.cpp
    src/utility/noncopyable.h
    src/record_management/record.cpp
    src/record_management/record.h
    src/record_management/table.h src/utility/binary_stream.h src/record_management/table.cpp src/utility/bitset_holder_type_selector.h)

add_executable(WaterySQL ${SOURCE_FILES})
