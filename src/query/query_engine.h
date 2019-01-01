//
// Created by Mike Smith on 2018-12-23.
//

#ifndef WATERYSQL_QUERY_ENGINE_H
#define WATERYSQL_QUERY_ENGINE_H

#include <memory>
#include <functional>

#include "../config/config.h"
#include "../record/table.h"
#include "../index/index_manager.h"
#include "../record/record_manager.h"
#include "../action/column_predicate.h"

#include "single_table_predicate.h"
#include "cross_table_predicate.h"

namespace watery {

class QueryEngine : public Singleton<QueryEngine> {

public:
    // hyper-parameters
    static constexpr auto table_scan_coeff = 3ull;
    static constexpr auto index_search_coeff = 10ull;
    static constexpr auto partial_index_scan_coeff = 2ull;
    static constexpr auto null_check_coeff = 2ull;
    static constexpr auto empty_result_coeff = 1ull;

private:
    static const Byte *_assemble_record(
        const std::shared_ptr<Table> &table, const Byte *raw,
        const uint16_t *sizes, const std::vector<ColumnOffset> &cols);
    
    static void _insert_record(
        const std::shared_ptr<Table> &table, const Byte *raw,
        const uint16_t *sizes, uint16_t count);
    
    static uint64_t _estimate_single_column_predicate_cost(
        const std::shared_ptr<Table> &t, ColumnOffset cid,
        PredicateOperator op, QueryPlan plan) noexcept;
    
    static QueryPlan _choose_column_query_plan(
        const std::shared_ptr<Table> &t, ColumnOffset cid, PredicateOperator op) noexcept;
    
    static void _delete_record(const std::shared_ptr<Table> &table, RecordOffset rid, const Byte *rec = nullptr);
    
    static bool _record_satisfies_predicates(
        const std::shared_ptr<Table> &table, const Byte *rec,
        const std::vector<SingleTablePredicate> &preds);
    
    static std::vector<SingleTablePredicate> _extract_single_table_predicates(
        const std::shared_ptr<Table> &table,
        const std::vector<ColumnPredicate> &preds);
    
    static std::vector<SingleTablePredicate> _extract_contextual_single_table_predicates(
        const std::shared_ptr<Table> &table,
        const std::vector<ColumnPredicate> &preds,
        const std::vector<std::shared_ptr<Table>> &ctx_tables,
        const std::vector<std::vector<Byte>> &ctx_records);
    
    static void _update_record(
        const std::shared_ptr<Table> &table, RecordOffset rid,
        const std::vector<ColumnOffset> &cols, const Byte *rec);
    
    static std::vector<RecordOffset> _gather_valid_single_table_record_offsets(
        const std::shared_ptr<Table> &table,
        const std::vector<SingleTablePredicate> &preds);
    
    static size_t _select_from_single_table(
        const std::shared_ptr<Table> &table,
        const std::vector<ColumnOffset> &cols,
        const std::vector<SingleTablePredicate> &preds,
        const std::function<void(const std::vector<std::string> &)> &receiver);
    
    static size_t _select_from_multiple_tables(
        const std::vector<std::string> &sel_tables,
        const std::vector<ColumnOffset> &sel_cols,
        std::vector<std::string> &src_tables,
        std::vector<std::shared_ptr<Table>> &ctx_tables,
        std::vector<std::vector<Byte>> &ctx_records,
        const std::vector<ColumnPredicate> &preds,
        const std::function<void(const std::vector<std::string> &)> &recv);
    
    static std::vector<std::string> _encode_selected_records(
        const std::vector<std::string> &selected_tables,
        const std::vector<ColumnOffset> &selected_cols,
        const std::vector<std::shared_ptr<Table>> &ctx_tables,
        const std::vector<std::vector<Byte>> &ctx_records);

protected:
    QueryEngine() = default;

public:
    size_t insert_records(
        const std::string &table_name, const std::vector<Byte> &raw,
        const std::vector<uint16_t> &field_sizes, const std::vector<uint16_t> &field_counts);
    
    size_t delete_records(const std::string &table_name, const std::vector<ColumnPredicate> &raw_preds);
    
    size_t update_records(
        const std::string &table_name, const std::vector<std::string> &columns, const std::vector<Byte> &values,
        const std::vector<uint16_t> &sizes, const std::vector<ColumnPredicate> &preds);
    
    size_t select_records(
        const std::vector<std::string> &sel_tables,
        const std::vector<std::string> &sel_columns,
        const std::vector<std::string> &src_tables,
        const std::vector<ColumnPredicate> &predicates,
        std::function<void(const std::vector<std::string> &)> receiver);
};

}

#endif  // WATERYSQL_QUERY_ENGINE_H
