//
// Created by Mike Smith on 2018-12-23.
//

#include <iostream>

#include "query_engine.h"
#include "../error/query_engine_error.h"
#include "../utility/memory/value_decoder.h"
#include "../utility/io/printer.h"

namespace watery {

const Byte *QueryEngine::_make_record(
    const RecordDescriptor &desc, const Byte *raw, const uint16_t *sizes, uint16_t count) {
    
    thread_local static std::vector<Byte> record_buffer;
    
    if (count > desc.field_count) {
        throw QueryEngineError{"Too many fields to insert."};
    }
    
    if (record_buffer.size() < desc.length) {
        record_buffer.resize(desc.length);
    }
    
    if (desc.null_mapped) {  // reset null field bitmap if any
        MemoryMapper::map_memory<NullFieldBitmap>(record_buffer.data()).set();
    }
    
    if (desc.reference_counted()) {  // reset ref count if any
        MemoryMapper::map_memory<uint32_t>(record_buffer.data() + sizeof(NullFieldBitmap)) = 0;
    }
    
    for (auto i = 0; i < count; i++) {
        auto size = sizes[i];
        if (size == 0) {
            if (!desc.field_descriptors[i].constraints.nullable()) {
                throw QueryEngineError{"Cannot assign NULL to a column that is not nullable."};
            }
            MemoryMapper::map_memory<NullFieldBitmap>(record_buffer.data())[i] = false;
        } else {
            ValueDecoder::decode(
                desc.field_descriptors[i].data_descriptor.type,
                std::string_view{raw, size},
                record_buffer.data() + desc.field_offsets[i]);
            raw += size;
        }
    }
    
    return record_buffer.data();
}

void QueryEngine::insert_records(
    const std::string &table_name, const std::vector<Byte> &raw,
    const std::vector<uint16_t> &field_sizes, const std::vector<uint16_t> &field_counts) {
    auto data_ptr = raw.data();
    auto size_ptr = field_sizes.data();
    auto table = _record_manager.open_table(table_name);
    auto count = 0u;
    for (auto &&field_count : field_counts) {
        _insert_record(table, data_ptr, size_ptr, field_count);
        for (auto i = 0; i < field_count; i++) {
            data_ptr += size_ptr[i];
        }
        size_ptr += field_count;
        Printer::print(std::cout, "Inserted ", ++count, "/", field_counts.size(), " rows...\n");
    }
}

void QueryEngine::_insert_record(
    const std::shared_ptr<Table> &table, const Byte *raw,
    const uint16_t *sizes, uint16_t count) {
    
    auto record = _make_record(table->descriptor(), raw, sizes, count);
    
    thread_local static std::vector<int32_t> indexed_column_indices;
    thread_local static std::vector<int32_t> foreign_key_column_indices;
    thread_local static std::vector<RecordOffset> foreign_record_offsets;
    thread_local static std::string string_buffer;
    indexed_column_indices.clear();
    foreign_key_column_indices.clear();
    const auto &desc = table->descriptor();
    RecordOffset rid{-1, -1};
    try {
        // first insert the record into the table.
        rid = table->insert_record(record);
        for (auto i = 0; i < desc.field_count; i++) {
            const auto &field_desc = desc.field_descriptors[i];
            auto p = record + desc.field_offsets[i];
            auto null = field_desc.constraints.nullable() && MemoryMapper::map_memory<NullFieldBitmap>(record)[i];
            if (field_desc.indexed && !null) {  // if indexed, insert into the index as well
                (string_buffer = table->name()).append(".").append(field_desc.name.data());
                auto index = _index_manager.open_index(string_buffer);
                _index_manager.insert_index_entry(index, p, rid);
                indexed_column_indices.emplace_back(i);  // save column index for rollbacks
            }
            if (field_desc.constraints.foreign() && !null) {  // update foreign key ref count if not null
                string_buffer.clear();
                string_buffer.append(field_desc.foreign_table_name.data()).append(".")
                             .append(field_desc.foreign_column_name.data());
                auto foreign_index = _index_manager.open_index(string_buffer);
                auto foreign_rid = _index_manager.search_unique_index_entry(foreign_index, p);
                auto foreign_table = _record_manager.open_table(field_desc.foreign_table_name.data());
                auto foreign_null_mapped = foreign_table->descriptor().null_mapped;
                foreign_table->update_record(foreign_rid, [foreign_null_mapped](Byte *old) {
                    if (foreign_null_mapped) {
                        old += sizeof(NullFieldBitmap);  // skip null map
                    }
                    MemoryMapper::map_memory<uint32_t>(old)++;  // inc ref count
                });
                foreign_key_column_indices.emplace_back(i);
                foreign_record_offsets.emplace_back(foreign_rid);  // save column index for rollbacks
            }
        }
    } catch (...) {
        auto e = std::current_exception();  // save exception ptr
        if (rid != RecordOffset{-1, -1}) {  // recover insertion into the table
            table->delete_record(rid);
        }
        for (auto &&i : indexed_column_indices) {  // recover insertion into indices
            auto &&field_desc = desc.field_descriptors[i];
            (string_buffer = table->name()).append(".").append(field_desc.name.data());
            auto index = _index_manager.open_index(string_buffer);
            _index_manager.delete_index_entry(index, record + desc.field_offsets[i], rid);
        }
        for (auto i = 0; i < foreign_key_column_indices.size(); i++) {  // recover foreign key ref count updates
            auto col = foreign_key_column_indices[i];
            auto foreign_rid = foreign_record_offsets[i];
            auto &field_desc = desc.field_descriptors[col];
            auto foreign_table = _record_manager.open_table(field_desc.foreign_table_name.data());
            auto foreign_null_mapped = foreign_table->descriptor().null_mapped;
            foreign_table->update_record(foreign_rid, [foreign_null_mapped](Byte *old) {
                if (foreign_null_mapped) {
                    old += sizeof(NullFieldBitmap);  // skip null map
                }
                MemoryMapper::map_memory<uint32_t>(old)--;  // dec ref count
            });
        }
        std::rethrow_exception(e);
    }
}

void QueryEngine::delete_record(const std::string &table_name, std::vector<ColumnPredicate> &predicates) {
    std::string_view empty{""};
    auto table = _record_manager.open_table(table_name);
    for (auto &&pred : predicates) {
        if (pred.table_name.data() == empty) {
            pred.table_name = table_name;
        }
    }
    predicates.erase(std::remove_if(predicates.begin(), predicates.end(), [&table_name](auto &&pred) {
        return pred.table_name.data() != table_name;
    }), predicates.end());
    std::sort(predicates.begin(), predicates.end(), [this](auto &&lhs, auto &&rhs) {
        return this->_estimate_predicate_cost(lhs) < this->_estimate_predicate_cost(rhs);
    });
    
}

uint64_t QueryEngine::_estimate_predicate_cost(const ColumnPredicate &predicate) {
    
    // the efficiency is estimated with respect to the following factors:
    //   unique, indexed, nullable, record_count, operator, field_length
    // and the cost are given as follows:
    //   c_unique = unique ? 1 : 2
    //   c_indexed = indexed ? 1 : 100
    //   c_record_count = record_count
    //   c_operator = case {
    //     EQUAL    -> 1
    //     UNEQUAL  -> 5
    //     LESS     -> 2
    //
    //   }
    
}

}
