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
    main.cpp
    filesystem_demo/bufmanager/BufferedPageManager.h
    filesystem_demo/bufmanager/FindReplace.h
    filesystem_demo/fileio/FileManager.h
    filesystem_demo/fileio/FileTable.h
    filesystem_demo/utils/compare.h
    filesystem_demo/utils/MyBitMap.h
    filesystem_demo/utils/MyHashMap.h
    filesystem_demo/utils/MyLinkList.h
    filesystem_demo/utils/pagedef.h
    filesystem_demo/testfilesystem.cpp
    record_management/record_manager.cpp
    record_management/record_manager.h
    config/config.h
    record_management/record_descriptor.h
    record_management/field_descriptor.h
    data_storage/type_tag.h
    data_storage/data_descriptor.h
    data_storage/data.h
    data_storage/data.cpp
    utility/noncopyable.h
    record_management/record.cpp
    record_management/record.h
    record_management/table.h utility/binary_stream.h record_management/table.cpp)

add_executable(WaterySQL ${SOURCE_FILES})
