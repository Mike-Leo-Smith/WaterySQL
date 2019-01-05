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
#include "../data/data_view.h"
#include "../error/cross_table_predicate_type_mismatch.h"
#include "../data/predicate_operator_helper.h"
#include "../utility/memory/value_string_padder.h"

namespace watery {

const Byte *QueryEngine::_assemble_record(
    const std::shared_ptr<Table> &table, const Byte *raw,
    const uint16_t *sizes, const std::vector<ColumnOffset> &cols) {
    
    const auto &desc = table->descriptor();
    if (desc.field_count < cols.size()) {
        throw TooManyColumnsToInsert{table->name(), static_cast<uint32_t>(cols.size())};
    }
    
    thread_local static std::vector<Byte> record_buffer;
    record_buffer.clear();
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
    
    std::vector<int32_t> indexed_column_indexes;
    std::vector<int32_t> foreign_key_column_indexes;
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
                indexed_column_indexes.emplace_back(i);  // save column index for rollbacks
            }
            if (field_desc.constraints.foreign() && !null) {  // update foreign key ref count if not null
                string_buffer.clear();
                string_buffer.append(field_desc.foreign_table_name.data()).append(".")
                             .append(field_desc.foreign_column_name.data());
                auto foreign_index = IndexManager::instance().open_index(string_buffer);
                auto foreign_rid = foreign_index->search_unique_index_entry(p);
                auto foreign_table = RecordManager::instance().open_table(field_desc.foreign_table_name.data());
                foreign_table->add_record_reference_count(foreign_rid);
                foreign_key_column_indexes.emplace_back(i);
                foreign_record_offsets.emplace_back(foreign_rid);  // save column index for rollbacks
            }
        }
    } catch (...) {
        auto e = std::current_exception();  // save exception ptr
        if (rid != RecordOffset{-1, -1}) {  // recover insertion into the table
            table->delete_record(rid);
        }
        for (auto &&i : indexed_column_indexes) {  // recover insertion into indexes
            auto &&field_desc = desc.field_descriptors[i];
            (string_buffer = table->name()).append(".").append(field_desc.name.data());
            auto index = IndexManager::instance().open_index(string_buffer);
            index->delete_index_entry(record + desc.field_offsets[i], rid);
        }
        for (auto i = 0; i < foreign_key_column_indexes.size(); i++) {  // recover foreign key ref count updates
            auto col = foreign_key_column_indexes[i];
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
    auto predicates = _extract_single_table_predicates(table, raw_preds);
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
    
    // TODO: ensure strong exception-safety
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

std::vector<SingleTablePredicate> QueryEngine::_extract_single_table_predicates(
    const std::shared_ptr<Table> &table,
    const std::vector<ColumnPredicate> &preds) {
    
    std::vector<SingleTablePredicate> predicates;
    for (auto &&raw_pred : preds) {
        if (!raw_pred.cross_table && raw_pred.table_name == table->name()) {
            auto cid = table->column_offset(raw_pred.column_name);
            auto plan = _choose_column_query_plan(table, cid, raw_pred.op);
            auto cost = _estimate_single_column_predicate_cost(table, cid, raw_pred.op, plan);
            auto data_desc = table->descriptor().field_descriptors[cid].data_descriptor;
            std::vector<Byte> operand;
            if (raw_pred.op != PredicateOperator::IS_NULL && raw_pred.op != PredicateOperator::NOT_NULL) {
                operand.resize(data_desc.length);
                ValueDecoder::decode(
                    data_desc.type,
                    std::string_view{raw_pred.operand.data(), raw_pred.operand.size()},
                    operand.data());
            }
            predicates.emplace_back(cid, raw_pred.op, std::move(operand), plan, cost);
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
    auto predicates = _extract_single_table_predicates(table, preds);
    auto rids = _gather_valid_single_table_record_offsets(table, predicates);
    for (auto &&rid : rids) {
        _update_record(table, rid, update_cols, update_rec);
    }
    
    return rids.size();
    
}

void QueryEngine::_update_record(
    const std::shared_ptr<Table> &table, RecordOffset rid,
    const std::vector<ColumnOffset> &cols, const Byte *rec) {
    
    // TODO: ensure strong exception-safety
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
                DataComparator cmp{field_desc.data_descriptor};
                const Byte *upper_bound = nullptr;
                const Byte *lower_bound = nullptr;
                for (auto &&pred : preds) {
                    if ((pred.op == PredicateOperator::GREATER || pred.op == PredicateOperator::GREATER_EQUAL) &&
                        (lower_bound == nullptr || cmp.greater(pred.operand.data(), lower_bound))) {
                        lower_bound = pred.operand.data();
                    } else if ((pred.op == PredicateOperator::LESS || pred.op == PredicateOperator::LESS_EQUAL) &&
                               (upper_bound == nullptr || cmp.less(pred.operand.data(), upper_bound))) {
                        upper_bound = pred.operand.data();
                    }
                }
                if (lower_bound != nullptr && upper_bound != nullptr && cmp.greater(lower_bound, upper_bound)) {
                    break;  // empty set
                }
                auto begin = lower_bound != nullptr ?
                             index->search_index_entry(lower_bound, {-1, -1}) :
                             index->index_entry_offset_begin();
                auto end = upper_bound != nullptr ?
                           index->next_index_entry_offset(index->search_index_entry(upper_bound, {max, max})) :
                           IndexEntryOffset{-1, -1};
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

size_t QueryEngine::select_records(
    const std::vector<std::string> &sel_tables,
    const std::vector<std::string> &sel_columns,
    const std::vector<std::string> &src_tables,
    const std::vector<ColumnPredicate> &predicates,
    std::function<void(const std::vector<std::string> &row)> receiver) {
    
    std::vector<ColumnOffset> cols;
    
    // single table selection
    if (src_tables.size() == 1) {
        cols.reserve(MAX_FIELD_COUNT);
        auto table = RecordManager::instance().open_table(src_tables[0]);
        if (sel_columns.empty()) {  // wildcard
            for (auto i = 0; i < table->descriptor().field_count; i++) { cols.emplace_back(i); }
        } else {
            for (auto &&c : sel_columns) { cols.emplace_back(table->column_offset(c)); }
        }
        return _select_from_single_table(table, cols, _extract_single_table_predicates(table, predicates), receiver);
    }
    
    // multi-table selection
    std::vector<std::shared_ptr<Table>> ctx_tables;
    std::vector<std::vector<Byte>> ctx_records;
    auto src = src_tables;
    
    // wildcard, gather all columns
    if (sel_tables.empty()) {
        std::vector<std::string> tables;
        tables.reserve(src_tables.size() * MAX_FIELD_COUNT);
        cols.reserve(src_tables.size() * MAX_FIELD_COUNT);
        for (auto &&src_table_name : src_tables) {
            auto src_table = RecordManager::instance().open_table(src_table_name);
            for (auto i = 0; i < src_table->descriptor().field_count; i++) {
                tables.emplace_back(src_table_name);
                cols.emplace_back(i);
            }
        }
        return _select_from_multiple_tables(tables, cols, src, ctx_tables, ctx_records, predicates, receiver);
    }
    
    // select columns
    cols.reserve(MAX_FIELD_COUNT);
    for (auto i = 0; i < sel_columns.size(); i++) {
        auto table = RecordManager::instance().open_table(sel_tables[i]);
        cols.emplace_back(table->column_offset(sel_columns[i]));
    }
    return _select_from_multiple_tables(sel_tables, cols, src, ctx_tables, ctx_records, predicates, receiver);
}

size_t QueryEngine::_select_from_single_table(
    const std::shared_ptr<Table> &table,
    const std::vector<ColumnOffset> &cols,
    const std::vector<SingleTablePredicate> &preds,
    const std::function<void(const std::vector<std::string> &)> &receiver) {
    
    auto &desc = table->descriptor();
    auto rids = _gather_valid_single_table_record_offsets(table, preds);
    static const std::string null_string{"NULL"};
    std::vector<std::string> row;
    for (auto &&rid : rids) {
        row.clear();
        auto rec = table->get_record(rid);
        for (auto &&col : cols) {
            auto &field_desc = desc.field_descriptors[col];
            auto field = field_desc.constraints.nullable() && MemoryMapper::map_memory<NullFieldBitmap>(rec)[col] ?
                         null_string :
                         DataView{field_desc.data_descriptor, rec + desc.field_offsets[col]}.to_string();
            row.emplace_back(std::move(field));
        }
        receiver(row);
    }
    return rids.size();
}

std::vector<SingleTablePredicate> QueryEngine::_extract_contextual_single_table_predicates(
    const std::shared_ptr<Table> &table, const std::vector<ColumnPredicate> &preds,
    const std::vector<std::shared_ptr<Table>> &ctx_tables, const std::vector<std::vector<Byte>> &ctx_records) {
    
    std::vector<SingleTablePredicate> predicates;
    
    auto get_rhs_operand = [](
        const std::shared_ptr<Table> &lhs_table, ColumnOffset lhs_cid,
        const std::shared_ptr<Table> &rhs_table, ColumnOffset rhs_cid,
        const Byte *rhs_rec) {
        
        std::vector<Byte> field;
        const auto &lhs_desc = lhs_table->descriptor();
        const auto &rhs_desc = rhs_table->descriptor();
        const auto &lhs_field_desc = lhs_desc.field_descriptors[lhs_cid];
        const auto &rhs_field_desc = rhs_desc.field_descriptors[rhs_cid];
        auto lhs_data_desc = lhs_field_desc.data_descriptor;
        auto rhs_data_desc = rhs_field_desc.data_descriptor;
        if (lhs_data_desc.type != rhs_data_desc.type) {
            throw CrossTablePredicateTypeMismatch{
                lhs_table->name(), lhs_field_desc.name.data(),
                rhs_table->name(), rhs_field_desc.name.data()};
        }
        if (!rhs_field_desc.constraints.nullable() || !MemoryMapper::map_memory<NullFieldBitmap>(rhs_rec)[rhs_cid]) {
            field.resize(lhs_data_desc.length);
            auto p = rhs_rec + rhs_desc.field_offsets[rhs_cid];
            std::memmove(field.data(), p, std::min(lhs_data_desc.length, rhs_data_desc.length));
        }
        return field;
    };
    
    for (auto &&pred : preds) {
        if (pred.table_name == table->name()) {  // as lhs
            if (pred.cross_table) {  // rhs is a table
                for (auto i = 0; i < ctx_tables.size(); i++) {  // find it if it is already in context
                    if (ctx_tables[i]->name() == pred.rhs_table_name) {
                        auto cid = table->column_offset(pred.column_name);
                        auto rhs_cid = ctx_tables[i]->column_offset(pred.rhs_column_name);
                        auto operand = get_rhs_operand(table, cid, ctx_tables[i], rhs_cid, ctx_records[i].data());
                        auto plan = operand.empty() ?
                                    QueryPlan::CONSTANT_EMPTY_RESULT :  // nulls make the pred constantly fail
                                    _choose_column_query_plan(table, cid, pred.op);
                        auto cost = _estimate_single_column_predicate_cost(table, cid, pred.op, plan);
                        predicates.emplace_back(cid, pred.op, std::move(operand), plan, cost);
                        break;
                    }
                }
            } else {  // rhs is a value
                std::vector<Byte> operand;
                auto cid = table->column_offset(pred.column_name);
                auto plan = _choose_column_query_plan(table, cid, pred.op);
                auto cost = _estimate_single_column_predicate_cost(table, cid, pred.op, plan);
                auto data_desc = table->descriptor().field_descriptors[cid].data_descriptor;
                if (pred.op != PredicateOperator::IS_NULL && pred.op != PredicateOperator::NOT_NULL) {
                    operand.resize(data_desc.length);
                    ValueDecoder::decode(
                        data_desc.type,
                        std::string_view{pred.operand.data(), pred.operand.size()},
                        operand.data());
                }
                predicates.emplace_back(cid, pred.op, std::move(operand), plan, cost);
            }
        } else if (pred.cross_table && pred.rhs_table_name == table->name()) {  // as rhs
            for (auto i = 0; i < ctx_tables.size(); i++) {  // find lhs table if any
                if (ctx_tables[i]->name() == pred.table_name) {
                    auto cid = table->column_offset(pred.rhs_column_name);
                    auto op = PredicateOperatorHelper::reversed(pred.op);
                    auto rhs_cid = ctx_tables[i]->column_offset(pred.column_name);
                    auto operand = get_rhs_operand(table, cid, ctx_tables[i], rhs_cid, ctx_records[i].data());
                    auto plan = operand.empty() ?
                                QueryPlan::CONSTANT_EMPTY_RESULT :  // skip nulls
                                _choose_column_query_plan(table, cid, op);
                    auto cost = _estimate_single_column_predicate_cost(table, cid, op, plan);
                    predicates.emplace_back(cid, op, std::move(operand), plan, cost);
                    break;
                }
            }
        }
    }
    std::sort(predicates.begin(), predicates.end());
    return predicates;
}

size_t QueryEngine::_select_from_multiple_tables(
    const std::vector<std::string> &sel_tables,
    const std::vector<ColumnOffset> &sel_cols,
    std::vector<std::string> &src_tables,
    std::vector<std::shared_ptr<Table>> &ctx_tables,
    std::vector<std::vector<Byte>> &ctx_records,
    const std::vector<ColumnPredicate> &preds,
    const std::function<void(const std::vector<std::string> &)> &recv) {
    
    if (ctx_tables.size() == src_tables.size()) {  // done
        recv(_encode_selected_records(sel_tables, sel_cols, ctx_tables, ctx_records));
        return 1;
    }
    
    auto ctx = ctx_tables.size();
    
    // find the best table to do selection first
    std::vector<std::string> sorted_src;
    sorted_src.reserve(src_tables.size());
    for (auto i = 0; i < ctx; i++) {
        sorted_src.emplace_back(src_tables[i]);
    }
    std::vector<SingleTablePredicate> predicates;
    auto min_table_index = 0ul;
    for (uint64_t i = ctx, min_score = std::numeric_limits<uint64_t>::max(); i < src_tables.size(); i++) {
        auto src_table = RecordManager::instance().open_table(src_tables[i]);
        auto pds = _extract_contextual_single_table_predicates(src_table, preds, ctx_tables, ctx_records);
        auto &desc = src_table->descriptor();
        auto score = pds.empty() ? table_scan_coeff * desc.length * src_table->record_count() : pds[0].cost;
        if (score <= min_score) {
            min_score = score;
            min_table_index = i;
            predicates = std::move(pds);
        }
    }
    sorted_src.emplace_back(src_tables[min_table_index]);
    for (auto i = ctx; i < min_table_index; i++) { sorted_src.emplace_back(src_tables[i]); }
    for (auto i = min_table_index + 1; i < src_tables.size(); i++) { sorted_src.emplace_back(src_tables[i]); }
    
    auto table = RecordManager::instance().open_table(sorted_src[ctx]);
    auto rids = _gather_valid_single_table_record_offsets(table, predicates);
    auto count = 0ul;
    ctx_tables.emplace_back(table);
    for (auto &&rid : rids) {
        std::vector<Byte> record;
        record.resize(table->descriptor().length);
        std::memmove(record.data(), table->get_record(rid), table->descriptor().length);
        ctx_records.emplace_back(std::move(record));
        count += _select_from_multiple_tables(sel_tables, sel_cols, sorted_src, ctx_tables, ctx_records, preds, recv);
        ctx_records.pop_back();
    }
    ctx_tables.pop_back();
    return count;
}

std::vector<std::string> QueryEngine::_encode_selected_records(
    const std::vector<std::string> &selected_tables,
    const std::vector<ColumnOffset> &selected_cols,
    const std::vector<std::shared_ptr<Table>> &ctx_tables,
    const std::vector<std::vector<Byte>> &ctx_records) {
    
    std::vector<std::string> row;
    static const std::string null_str{"NULL"};
    for (auto i = 0; i < selected_cols.size(); i++) {
        for (auto ctx = 0; ctx < ctx_tables.size(); ctx++) {
            if (ctx_tables[ctx]->name() == selected_tables[i]) {
                auto col = selected_cols[i];
                const auto &desc = ctx_tables[ctx]->descriptor();
                const auto &field_desc = desc.field_descriptors[col];
                auto data_desc = field_desc.data_descriptor;
                const auto rec = ctx_records[ctx].data();
                auto null = field_desc.constraints.nullable() && MemoryMapper::map_memory<NullFieldBitmap>(rec)[col];
                auto field_str = null ?
                                 null_str :
                                 DataView{field_desc.data_descriptor, rec + desc.field_offsets[col]}.to_string();
                row.emplace_back(std::move(field_str));
                break;
            }
        }
    }
    return row;
}
    
}
