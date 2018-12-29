//
// Created by Mike Smith on 2018-12-23.
//

#include <cstring>
#include <iostream>

#include "query_engine.h"
#include "single_table_predicate.h"
#include "../utility/memory/value_decoder.h"
#include "../utility/io/printer.h"
#include "../error/too_many_columns_to_insert.h"
#include "../error/invalid_null_field.h"
#include "../error/negative_record_reference_count.h"
#include "../error/deleting_referenced_record.h"
#include "../error/conflict_record_field_update.h"
#include "../error/invalid_primary_key_update.h"

namespace watery {

const Byte *QueryEngine::_make_record(
    const std::shared_ptr<Table> &table, const Byte *raw, const uint16_t *sizes, uint16_t count) {
    
    const auto &desc = table->descriptor();
    if (desc.field_count < count) {
        throw TooManyColumnsToInsert{table->name(), count};
    }
    
    thread_local static std::vector<Byte> record_buffer;
    if (record_buffer.size() < desc.length) {
        record_buffer.resize(desc.length);
    }
    
    if (desc.null_mapped) {  // reset null field bitmap if any
        MemoryMapper::map_memory<NullFieldBitmap>(record_buffer.data()).set();
    }
    if (desc.reference_counted) {  // reset ref count if any
        MemoryMapper::map_memory<uint32_t>(record_buffer.data() + sizeof(NullFieldBitmap)) = 0;
    }
    
    for (auto i = 0; i < count; i++) {
        auto size = sizes[i];
        if (size == 0) {
            if (!desc.field_descriptors[i].constraints.nullable()) {
                throw InvalidNullField{table->name(), desc.field_descriptors[i].name.data()};
            }
            MemoryMapper::map_memory<NullFieldBitmap>(record_buffer.data())[i] = false;
        } else {
            ValueDecoder::decode(
                desc.field_descriptors[i].data_descriptor.type,
                std::string_view{raw, size},
                record_buffer.data() + desc.field_offsets[i],
                desc.field_descriptors[i].data_descriptor.length);
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
    auto table = RecordManager::instance().open_table(table_name);
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
    
    auto record = _make_record(table, raw, sizes, count);
    
    std::vector<int32_t> indexed_column_indices;
    std::vector<int32_t> foreign_key_column_indices;
    std::vector<RecordOffset> foreign_record_offsets;
    std::string string_buffer;
    const auto &desc = table->descriptor();
    RecordOffset rid{-1, -1};
    try {
        // first insert the record into the table.
        rid = table->insert_record(record);
        for (auto i = 0; i < desc.field_count; i++) {
            const auto &field_desc = desc.field_descriptors[i];
            auto p = record + desc.field_offsets[i];
            auto null = field_desc.constraints.nullable() && MemoryMapper::map_memory<NullFieldBitmap>(record)[i];
            if (field_desc.indexed && !null) {  // if indexed and not null, insert into the index as well
                (string_buffer = table->name()).append(".").append(field_desc.name.data());
                auto index = IndexManager::instance().open_index(string_buffer);
                index->insert_index_entry(p, rid);
                indexed_column_indices.emplace_back(i);  // save column index for rollbacks
            }
            if (field_desc.constraints.foreign() && !null) {  // update foreign key ref count if not null
                string_buffer.clear();
                string_buffer.append(field_desc.foreign_table_name.data()).append(".")
                             .append(field_desc.foreign_column_name.data());
                auto foreign_index = IndexManager::instance().open_index(string_buffer);
                auto foreign_rid = foreign_index->search_unique_index_entry(p);
                auto foreign_table = RecordManager::instance().open_table(field_desc.foreign_table_name.data());
                foreign_table->add_record_reference_count(foreign_rid);
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
            auto index = IndexManager::instance().open_index(string_buffer);
            index->delete_index_entry(record + desc.field_offsets[i], rid);
        }
        for (auto i = 0; i < foreign_key_column_indices.size(); i++) {  // recover foreign key ref count updates
            auto col = foreign_key_column_indices[i];
            auto foreign_rid = foreign_record_offsets[i];
            auto &field_desc = desc.field_descriptors[col];
            auto foreign_table = RecordManager::instance().open_table(field_desc.foreign_table_name.data());
            foreign_table->drop_record_reference_count(foreign_rid);
        }
        std::rethrow_exception(e);
    }
}

void QueryEngine::delete_records(const std::string &table_name, const std::vector<ColumnPredicate> &raw_preds) {
    
    auto table = RecordManager::instance().open_table(table_name);
    auto predicates = _simplify_single_table_column_predicates(table, raw_preds);
    
    // brute force is ok
    for (auto rid = table->record_offset_begin();
         !table->is_record_offset_end(rid);
         rid = table->next_record_offset(rid)) {
        auto rec = table->get_record(rid);
        if (_record_satisfies_predicates(table, rec, predicates)) {
            _delete_record(table, rid, rec);
        }
    }
    
}

uint64_t QueryEngine::_estimate_predicate_cost(
    const std::shared_ptr<Table> &t, ColumnOffset cid, PredicateOperator op) noexcept {
    
    // hyper-parameters
    constexpr auto table_scan_coeff = 3ull;
    constexpr auto index_search_coeff = 8ull;
    constexpr auto partial_index_scan_coeff = 2ull;
    constexpr auto null_check_coeff = 2ull;
    constexpr auto empty_result_coeff = 1ull;
    
    const auto &field_desc = t->descriptor().field_descriptors[cid];
    auto len = field_desc.data_descriptor.length;
    auto count = t->record_count();
    
    switch (op) {
        case PredicateOperator::EQUAL:
            // index search * compare if indexed, otherwise full table scan * compare
            return field_desc.indexed ? index_search_coeff * len : table_scan_coeff * count * len;
        case PredicateOperator::UNEQUAL:
            // full table scan * compare
            return table_scan_coeff * len * count;
        case PredicateOperator::LESS:
        case PredicateOperator::LESS_EQUAL:
        case PredicateOperator::GREATER:
        case PredicateOperator::GREATER_EQUAL:
            // (index search + partial index scan) * compare if indexed, otherwise full table scan * compare
            return field_desc.indexed ?
                   (index_search_coeff + partial_index_scan_coeff * count) * len :
                   table_scan_coeff * len * count;
        case PredicateOperator::IS_NULL:
            // full table scan * null check if nullable, otherwise empty result
            return field_desc.constraints.nullable() ?
                   table_scan_coeff * count * null_check_coeff :
                   empty_result_coeff;
        case PredicateOperator::NOT_NULL:
            // full table scan * null check
            return table_scan_coeff * count * null_check_coeff;
        default:
            // should not occur
            return std::numeric_limits<uint64_t>::max();
    }
    
}

void QueryEngine::_delete_record(const std::shared_ptr<Table> &table, RecordOffset rid, const Byte *rec) {
    
    if (rec == nullptr) {
        rec = table->get_record(rid);
    }
    
    const auto &rec_desc = table->descriptor();
    
    // check primary key constraint
    if (rec_desc.reference_counted &&
        MemoryMapper::map_memory<uint32_t>(rec_desc.null_mapped ? rec + sizeof(NullFieldBitmap) : rec) != 0) {
        throw DeletingReferencedRecord{table->name()};
    }
    
    for (auto i = 0; i < rec_desc.field_count; i++) {
        auto &field_desc = rec_desc.field_descriptors[i];
        auto field = rec + rec_desc.field_offsets[i];
        
        // skip if null
        if (field_desc.constraints.nullable() &&
            MemoryMapper::map_memory<NullFieldBitmap>(rec)[i]) {
            continue;
        }
        
        // remove from index
        if (field_desc.indexed) {
            auto index_name = std::string{table->name()}.append(".").append(field_desc.name.data());
            auto index = IndexManager::instance().open_index(index_name);
            index->delete_index_entry(field, rid);
        }
        
        // drop foreign record ref count
        if (field_desc.constraints.foreign()) {
            std::string name{field_desc.foreign_table_name.data()};
            auto foreign_table = RecordManager::instance().open_table(name);
            name.append(".").append(field_desc.foreign_column_name.data());
            auto foreign_index = IndexManager::instance().open_index(name);
            auto foreign_rid = foreign_index->search_unique_index_entry(field);
            foreign_table->drop_record_reference_count(foreign_rid);
        }
    }
    
    // remove it from the table...
    table->delete_record(rid);
}

bool QueryEngine::_record_satisfies_predicates(
    const std::shared_ptr<Table> &table, const Byte *rec,
    const std::vector<SingleTablePredicate> &preds) {
    
    for (auto &&pred : preds) {
        
        if (pred.table->name() != table->name()) {
            continue;
        }
        
        auto &&field_desc = table->descriptor().field_descriptors[pred.column_offset];
        auto &&data_desc = field_desc.data_descriptor;
        auto &&field = rec + table->descriptor().field_offsets[pred.column_offset];
        DataComparator cmp{data_desc};
        switch (pred.op) {
            case PredicateOperator::EQUAL:
                if (cmp.unequal(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::UNEQUAL:
                if (cmp.equal(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::LESS:
                if (cmp.greater_equal(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::LESS_EQUAL:
                if (cmp.greater(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::GREATER:
                if (cmp.less_equal(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::GREATER_EQUAL:
                if (cmp.less(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::IS_NULL:
                if (!field_desc.constraints.nullable() ||
                    !MemoryMapper::map_memory<NullFieldBitmap>(rec)[pred.column_offset]) {
                    return false;
                }
                break;
            case PredicateOperator::NOT_NULL:
                if (field_desc.constraints.nullable() &&
                    MemoryMapper::map_memory<NullFieldBitmap>(rec)[pred.column_offset]) {
                    return false;
                }
                break;
        }
    }
    
    return true;
}

std::vector<SingleTablePredicate> QueryEngine::_simplify_single_table_column_predicates(
    const std::shared_ptr<Table> &table,
    const std::vector<ColumnPredicate> &preds) {
    
    std::vector<SingleTablePredicate> predicates;
    for (auto &&raw_pred : preds) {
        if (!raw_pred.cross_table && (raw_pred.table_name.empty() || raw_pred.table_name == table->name())) {
            auto cid = table->column_offset(raw_pred.column_name);
            predicates.emplace_back(
                table, cid, raw_pred.op, raw_pred.operand,
                _estimate_predicate_cost(table, cid, raw_pred.op));
        }
    }
    std::sort(predicates.begin(), predicates.end());
    
    return predicates;
    
}

void QueryEngine::update_records(
    const std::string &table_name,
    const std::vector<std::string> &columns,
    const std::vector<Byte> &values,
    const std::vector<uint16_t> &sizes,
    const std::vector<ColumnPredicate> &preds) {
    
    auto table = RecordManager::instance().open_table(table_name);
    auto &desc = table->descriptor();
    
    std::vector<ColumnOffset> update_cols;
    std::vector<Byte> update_rec;
    
    update_rec.resize(table->descriptor().length);
    if (desc.null_mapped) {
        MemoryMapper::map_memory<NullFieldBitmap>(update_rec.data()).reset();
    }
    
    auto p = values.data();
    for (auto i = 0; i < columns.size(); i++) {
        auto &field_desc = desc.field_descriptors[i];
        auto cid = table->column_offset(columns[i]);
        if (cid == table->descriptor().primary_key_column_offset) {
            throw InvalidPrimaryKeyUpdate{table_name, columns[i]};
        }
        if (std::find(update_cols.cbegin(), update_cols.cend(), cid) != update_cols.cend()) {
            throw ConflictRecordFieldUpdate{table_name, columns[i]};
        }
        update_cols.emplace_back(cid);
        if (sizes[i] == 0) {  // null
            if (!field_desc.constraints.nullable()) {
                throw InvalidNullField{table_name, columns[i]};
            }
            MemoryMapper::map_memory<NullFieldBitmap>(update_rec.data())[i] = true;
        } else {  // not null
            auto &data_desc = field_desc.data_descriptor;
            ValueDecoder::decode(
                data_desc.type,
                std::string_view{p, sizes[i]},
                update_rec.data() + desc.field_offsets[i],
                data_desc.length);
        }
    }
    
    auto predicates = _simplify_single_table_column_predicates(table, preds);
    
    for (auto rid = table->record_offset_begin();
         !table->is_record_offset_end(rid);
         rid = table->next_record_offset(rid)) {
        if (_record_satisfies_predicates(table, table->get_record(rid), predicates)) {
            _update_record(table, rid, update_cols, update_rec.data());
        }
    }
    
}

void QueryEngine::_update_record(
    const std::shared_ptr<Table> &table, RecordOffset rid,
    const std::vector<ColumnOffset> &cols, const Byte *rec) {
    
    for (auto &&cid : cols) {
        table->update_record(rid, [c = cid, t = table, r = rec, rid](Byte *old) {
            auto &field_desc = t->descriptor().field_descriptors[c];
            auto offset = t->descriptor().field_offsets[c];
            auto data_desc = field_desc.data_descriptor;
            DataComparator cmp{data_desc};
            auto old_field = old + offset;
            auto new_field = r + offset;
            if (cmp.equal(old_field, new_field)) { return; }
            if (field_desc.constraints.nullable()) {
                MemoryMapper::map_memory<NullFieldBitmap>(old)[c] = MemoryMapper::map_memory<NullFieldBitmap>(r)[c];
            }
            if (field_desc.indexed) {
                auto index_name = (t->name() + ".").append(field_desc.name.data());
                auto index = IndexManager::instance().open_index(index_name);
                index->delete_index_entry(old_field, rid);
                index->insert_index_entry(new_field, rid);
            }
            if(field_desc.constraints.foreign()) {
                std::string name{field_desc.foreign_table_name.data()};
                auto foreign_table = RecordManager::instance().open_table(name);
                name.append(".").append(field_desc.foreign_column_name.data());
                auto foreign_index = IndexManager::instance().open_index(name);
                auto old_foreign_rid = foreign_index->search_unique_index_entry(old_field);
                foreign_table->drop_record_reference_count(old_foreign_rid);
                auto new_foreign_rid = foreign_index->search_unique_index_entry(new_field);
                foreign_table->add_record_reference_count(new_foreign_rid);
            }
            std::memmove(old_field, new_field, data_desc.length);
        });
    }
    
}
    
}
