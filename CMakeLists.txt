cmake_minimum_required(VERSION 3.12)

project(WaterySQL C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

set(COMPILE_WITH_CLANG 1)

if (COMPILE_WITH_CLANG)
    set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
    link_directories(/usr/local/opt/llvm/lib)
    link_libraries(c++fs)
else ()
    set(CMAKE_C_COMPILER gcc-8)
    set(CMAKE_CXX_COMPILER g++-8)
    link_libraries(stdc++fs)
endif ()

set(CMAKE_C_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -mtune=native -fomit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast -march=native -mtune=native -fomit-frame-pointer")

set(SOURCE_FILES
    src/config/config.h
    src/data/record_descriptor.h
    src/data/field_descriptor.h
    src/data/type_tag.h
    src/data/data_descriptor.h
    src/error/page_manager_error.h
    src/index/index_manager.cpp
    src/index/index_manager.h
    src/page/page_manager.cpp
    src/page/page_manager.h
    src/utility/type/non_copyable.h
    src/utility/type/non_movable.h
    src/utility/type/singleton.h
    src/utility/io/error_printer.h
    src/utility/io/printer.h
    src/error/error.h
    src/utility/type/non_constructible.h
    src/record/record_offset.h
    src/record/record_manager.cpp
    src/record/record_manager.h
    src/record/table.h
    src/record/data_page_header.h
    src/utility/memory/memory_mapper.h
    src/record/table_header.h
    src/record/data_page.h
    src/index/index_node.h
    src/index/index.h
    src/index/index_header.h
    src/index/index_node_header.h
    src/index/index_entry_offset.h
    src/index/index_node_link.h
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
    src/utility/io/file_reader.h
    src/utility/memory/string_view_copier.h
    src/data/data_view.h
    src/utility/memory/value_decoder.h
    src/error/value_decoder_error.h
    src/utility/time/date_validator.h
    src/action/column_predicate.h
    src/action/delete_record_actor.h
    src/data/predicate_operator.h
    src/data/predicate_operator_helper.h
    src/action/update_record_actor.h
    src/action/select_record_actor.h
    src/utility/memory/identifier_comparison.h
    src/page/page_handle.h
    src/action/exit_actor.h
    src/utility/io/record_descriptor_printer.h
    src/query/query_engine.cpp
    src/query/query_engine.h
    src/data/type_tag_helper.h
    src/action/execute_file_actor.h
    src/query/single_table_predicate.h src/record/table.cpp
    src/error/record_offset_out_of_range.h
    src/error/record_not_found.h
    src/error/record_slot_usage_bitmap_corrupt.h
    src/error/negative_foreign_key_reference_count.h
    src/error/record_oversized.h
    src/error/closing_shared_table.h
    src/index/index.cpp
    src/error/empty_index_tree.h
    src/error/index_entry_offset_out_of_range.h
    src/error/index_entry_not_found.h
    src/error/unique_search_in_non_unique_index.h
    src/error/conflict_index_entry_insertion.h
    src/error/closing_shared_index.h
    src/error/index_entry_oversized.h
    src/error/too_many_columns_to_insert.h
    src/error/invalid_null_field.h
    src/error/field_not_found.h
    src/error/negative_record_reference_count.h
    src/error/deleting_referenced_record.h
    src/error/conflict_record_field_update.h
    src/error/invalid_primary_key_update.h
    src/error/record_reference_not_counted.h
    src/utility/io/column_predicate_printer.h
    src/query/query_plan.h
    src/query/cross_table_predicate.h
    src/query/selection_context.h
    src/error/cross_table_predicate_type_mismatch.h
    src/data/aggregate_function.h
    src/data/aggregate_function_helper.h
    src/utility/memory/value_string_padder.h src/utility/io/html_table_printer.cpp src/utility/io/html_table_printer.h src/action/commit_actor.h)

add_executable(WaterySQL ${SOURCE_FILES} src/main.cpp)
add_executable(WaterySQL-IndexManagerTest ${SOURCE_FILES} test/index_manager_test.cpp)
add_executable(WaterySQL-CommandParsingTest ${SOURCE_FILES} test/command_parsing_test.cpp)

add_library(WaterySQLEngine SHARED ${SOURCE_FILES} lib/watery_sql.cpp lib/watery_sql.h)

find_package(JNI REQUIRED)
if (JNI_FOUND)
    include_directories(${JNI_INCLUDE_DIRS})
    add_library(WaterySQLEngineJNI SHARED ${SOURCE_FILES} lib/EngineJNI.h lib/EngineJNI.cpp)
    target_link_libraries(WaterySQLEngineJNI ${JNI_LIBRARIES})
endif ()
