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

const Byte *QueryEngine::_assemble_record(
    const std::shared_ptr<Table> &table, const Byte *raw,
    const uint16_t *sizes, const std::vector<ColumnOffset> &cols) {
    
    const auto &desc = table->descriptor();
    if (desc.field_count < cols.size()) {
        throw TooManyColumnsToInsert{table->name(), static_cast<uint32_t>(cols.size())};
    }
    
    thread_local static std::vector<Byte> record_buffer;
    record_buffer.resize(desc.length);
    
    if (desc.null_mapped) {  // reset null field bitmap if any
        MemoryMapper::map_memory<NullFieldBitmap>(record_buffer.data()).set();
    }
    if (desc.reference_counted) {  // reset ref count if any
        MemoryMapper::map_memory<uint32_t>(record_buffer.data() + sizeof(NullFieldBitmap)) = 0;
    }
    
    for (auto i = 0; i < cols.size(); i++) {
        auto size = sizes[i];
        auto col = cols[i];
        auto &field_desc = desc.field_descriptors[col];
        if (size == 0) {  // null
            if (!field_desc.constraints.nullable()) {
                throw InvalidNullField{table->name(), field_desc.name.data()};
            }
        } else {  // not null
            auto data_desc = field_desc.data_descriptor;
            ValueDecoder::decode(
                data_desc.type,
                std::string_view{raw, size},
                record_buffer.data() + desc.field_offsets[col],
                data_desc.length);
            raw += size;
            MemoryMapper::map_memory<NullFieldBitmap>(record_buffer.data())[col] = false;
        }
    }
    
    return record_buffer.data();
}

size_t QueryEngine::insert_records(
    const std::string &table_name, const std::vector<Byte> &raw,
    const std::vector<uint16_t> &field_sizes, const std::vector<uint16_t> &field_counts) {
    auto data_ptr = raw.data();
    auto size_ptr = field_sizes.data();
    auto table = RecordManager::instance().open_table(table_name);
    for (auto &&field_count : field_counts) {
        _insert_record(table, data_ptr, size_ptr, field_count);
        for (auto i = 0; i < field_count; i++) {
            data_ptr += size_ptr[i];
        }
        size_ptr += field_count;
    }
    return field_counts.size();
}

void QueryEngine::_insert_record(
    const std::shared_ptr<Table> &table, const Byte *raw,
    const uint16_t *sizes, uint16_t count) {
    
    thread_local static std::vector<ColumnOffset> columns;
    columns.clear();
    
    for (auto i = 0; i < count; i++) {
        columns.emplace_back(i);
    }
    auto record = _assemble_record(table, raw, sizes, columns);
    
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

size_t QueryEngine::delete_records(const std::string &table_name, const std::vector<ColumnPredicate> &raw_preds) {
    
    auto table = RecordManager::instance().open_table(table_name);
    auto predicates = _extract_single_table_column_predicates(table, raw_preds);
    auto rids = _gather_valid_single_table_record_offsets(table, predicates);
    
    // delete in a reversed order to reduce data copy
    for (auto it = rids.crbegin(); it != rids.crend(); it++) {
        _delete_record(table, *it, table->get_record(*it));
    }
    
    return rids.size();
}

uint64_t QueryEngine::_estimate_single_column_predicate_cost(
    const std::shared_ptr<Table> &t, ColumnOffset cid,
    PredicateOperator op, QueryPlan plan) noexcept {
    
    // hyper-parameters
    constexpr auto table_scan_coeff = 3ull;
    constexpr auto index_search_coeff = 8ull;
    constexpr auto partial_index_scan_coeff = 2ull;
    constexpr auto null_check_coeff = 2ull;
    constexpr auto empty_result_coeff = 1ull;
    
    const auto &field_desc = t->descriptor().field_descriptors[cid];
    auto len = field_desc.data_descriptor.length;
    auto count = t->record_count();
    
    switch (plan) {
        case QueryPlan::CONSTANT_EMPTY_RESULT:
            return empty_result_coeff;
        case QueryPlan::FULL_TABLE_SCAN_AND_COMPARE:
            return table_scan_coeff * count * len;
        case QueryPlan::FULL_TABLE_SCAN_AND_NULL_CHECK:
            return table_scan_coeff * null_check_coeff * count;
        case QueryPlan::INDEX_SEARCH_AND_COMPARE:
            return index_search_coeff * len;
        case QueryPlan::PARTIAL_INDEX_SCAN_AND_COMPARE:
            return (index_search_coeff + partial_index_scan_coeff * count) * len;
        default:
            return table_scan_coeff * count * len;
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
        
        auto cid = pred.column_offset;
        auto &&field_desc = table->descriptor().field_descriptors[cid];
        auto &&data_desc = field_desc.data_descriptor;
        auto &&field = rec + table->descriptor().field_offsets[cid];
        auto null = field_desc.constraints.nullable() && MemoryMapper::map_memory<NullFieldBitmap>(rec)[cid];
        DataComparator cmp{data_desc};
        switch (pred.op) {
            case PredicateOperator::EQUAL:
                if (null || cmp.unequal(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::UNEQUAL:
                if (null || cmp.equal(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::LESS:
                if (null || cmp.greater_equal(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::LESS_EQUAL:
                if (null || cmp.greater(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::GREATER:
                if (null || cmp.less_equal(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::GREATER_EQUAL:
                if (null || cmp.less(field, pred.operand.data())) { return false; }
                break;
            case PredicateOperator::IS_NULL:
                if (!null) { return false; }
                break;
            case PredicateOperator::NOT_NULL:
                if (null) { return false; }
                break;
        }
    }
    
    return true;
}

std::vector<SingleTablePredicate> QueryEngine::_extract_single_table_column_predicates(
    const std::shared_ptr<Table> &table,
    const std::vector<ColumnPredicate> &preds) {
    
    std::vector<SingleTablePredicate> predicates;
    for (auto &&raw_pred : preds) {
        if (!raw_pred.cross_table && raw_pred.table_name == table->name()) {
            auto cid = table->column_offset(raw_pred.column_name);
            auto plan = _choose_column_query_plan(table, cid, raw_pred.op);
            auto cost = _estimate_single_column_predicate_cost(table, cid, raw_pred.op, plan);
            predicates.emplace_back(table, cid, raw_pred.op, raw_pred.operand, plan, cost);
        }
    }
    std::sort(predicates.begin(), predicates.end());
    
    return predicates;
    
}

size_t QueryEngine::update_records(
    const std::string &table_name,
    const std::vector<std::string> &columns,
    const std::vector<Byte> &values,
    const std::vector<uint16_t> &sizes,
    const std::vector<ColumnPredicate> &preds) {
    
    auto table = RecordManager::instance().open_table(table_name);
    auto &desc = table->descriptor();
    
    thread_local static std::vector<ColumnOffset> update_cols;
    update_cols.clear();
    
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
    }
    
    auto update_rec = _assemble_record(table, values.data(), sizes.data(), update_cols);
    auto predicates = _extract_single_table_column_predicates(table, preds);
    auto rids = _gather_valid_single_table_record_offsets(table, predicates);
    for (auto &&rid : rids) {
        _update_record(table, rid, update_cols, update_rec);
    }
    
    return rids.size();
    
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
            auto nullable = field_desc.constraints.nullable();
            auto old_null = nullable && MemoryMapper::map_memory<NullFieldBitmap>(old)[c];
            auto new_null = nullable && MemoryMapper::map_memory<NullFieldBitmap>(r)[c];
            if (nullable) {
                MemoryMapper::map_memory<NullFieldBitmap>(old)[c] = new_null;
            }
            if (!old_null || !new_null) {
                if (field_desc.indexed) {
                    auto index_name = (t->name() + ".").append(field_desc.name.data());
                    auto index = IndexManager::instance().open_index(index_name);
                    if (!old_null) { index->delete_index_entry(old_field, rid); }
                    if (!new_null) { index->insert_index_entry(new_field, rid); }
                }
                if (field_desc.constraints.foreign()) {
                    std::string name{field_desc.foreign_table_name.data()};
                    auto foreign_table = RecordManager::instance().open_table(name);
                    name.append(".").append(field_desc.foreign_column_name.data());
                    auto foreign_index = IndexManager::instance().open_index(name);
                    if (!old_null) {
                        foreign_table->drop_record_reference_count(foreign_index->search_unique_index_entry(old_field));
                    }
                    if (!new_null) {
                        foreign_table->add_record_reference_count(foreign_index->search_unique_index_entry(new_field));
                    }
                }
            }
            std::memmove(old_field, new_field, data_desc.length);
        });
    }
    
}

QueryPlan QueryEngine::_choose_column_query_plan(
    const std::shared_ptr<Table> &t, ColumnOffset cid, PredicateOperator op) noexcept {
    
    const auto &field_desc = t->descriptor().field_descriptors[cid];
    
    switch (op) {
        case PredicateOperator::EQUAL:
            // index search * compare if indexed, otherwise full table scan * compare
            return field_desc.indexed ? QueryPlan::INDEX_SEARCH_AND_COMPARE : QueryPlan::FULL_TABLE_SCAN_AND_COMPARE;
        case PredicateOperator::UNEQUAL:
            // full table scan * compare
            return QueryPlan::FULL_TABLE_SCAN_AND_COMPARE;
        case PredicateOperator::LESS:
        case PredicateOperator::LESS_EQUAL:
        case PredicateOperator::GREATER:
        case PredicateOperator::GREATER_EQUAL:
            // (index search + partial index scan) * compare if indexed, otherwise full table scan * compare
            return field_desc.indexed ?
                   QueryPlan::PARTIAL_INDEX_SCAN_AND_COMPARE :
                   QueryPlan::FULL_TABLE_SCAN_AND_COMPARE;
        case PredicateOperator::IS_NULL:
            // full table scan * null check if nullable, otherwise empty result
            return field_desc.constraints.nullable() ?
                   QueryPlan::FULL_TABLE_SCAN_AND_NULL_CHECK :
                   QueryPlan::CONSTANT_EMPTY_RESULT;
        case PredicateOperator::NOT_NULL:
            // full table scan * null check
            return QueryPlan::FULL_TABLE_SCAN_AND_NULL_CHECK;
        default:
            // should not occur
            return QueryPlan::FULL_TABLE_SCAN_AND_COMPARE;
    }
}

std::vector<RecordOffset> QueryEngine::_gather_valid_single_table_record_offsets(
    const std::shared_ptr<Table> &table, const std::vector<SingleTablePredicate> &preds) {
    
    std::vector<RecordOffset> rids;
    if (preds.empty()) {  // all
        for (auto rid = table->record_offset_begin();
             !table->is_record_offset_end(rid);
             rid = table->next_record_offset(rid)) {
            rids.emplace_back(rid);
        }
    } else {
        auto &&field_desc = table->descriptor().field_descriptors[preds[0].column_offset];
        auto data = preds[0].operand.data();
        auto max = std::numeric_limits<PageOffset>::max();
        switch (preds[0].query_plan) {
            case QueryPlan::CONSTANT_EMPTY_RESULT:
                break;
            case QueryPlan::FULL_TABLE_SCAN_AND_COMPARE:
            case QueryPlan::FULL_TABLE_SCAN_AND_NULL_CHECK:
                for (auto rid = table->record_offset_begin();
                     !table->is_record_offset_end(rid);
                     rid = table->next_record_offset(rid)) {
                    if (auto rec = table->get_record(rid); _record_satisfies_predicates(table, rec, preds)) {
                        rids.emplace_back(rid);
                    }
                }
                break;
            case QueryPlan::INDEX_SEARCH_AND_COMPARE: {  // only in equal
                auto index_name = (table->name() + ".").append(field_desc.name.data());
                auto index = IndexManager::instance().open_index(index_name);
                auto begin = index->search_index_entry(data, {-1, -1});
                auto end = index->next_index_entry_offset(index->search_index_entry(data, {max, max}));
                for (auto it = begin; it != end; it = index->next_index_entry_offset(it)) {
                    auto rid = index->related_record_offset(it);
                    auto rec = table->get_record(rid);
                    if (_record_satisfies_predicates(table, rec, preds)) { rids.emplace_back(rid); }
                }
                break;
            }
            case QueryPlan::PARTIAL_INDEX_SCAN_AND_COMPARE: {
                auto index_name = (table->name() + ".").append(field_desc.name.data());
                auto index = IndexManager::instance().open_index(index_name);
                IndexEntryOffset begin{-1, -1};
                IndexEntryOffset end{-1, -1};
                IndexEntryOffset tmp{-1, -1};
                if (preds[0].op == PredicateOperator::GREATER || preds[0].op == PredicateOperator::GREATER_EQUAL) {
                    begin = index->search_index_entry(data, {-1, -1});
                } else if (preds[0].op == PredicateOperator::LESS || preds[0].op == PredicateOperator::LESS_EQUAL) {
                    begin = index->index_entry_offset_begin();
                    tmp = index->search_index_entry(data, {max, max});
                    end = index->next_index_entry_offset(tmp);
                }
                for (auto it = begin; it != end; it = index->next_index_entry_offset(it)) {
                    auto rid = index->related_record_offset(it);
                    auto rec = table->get_record(rid);
                    if (_record_satisfies_predicates(table, rec, preds)) { rids.emplace_back(rid); }
                }
                break;
            }
            default:  // should not occur
                break;
        }
    }
    
    return rids;
    
}
    
}
