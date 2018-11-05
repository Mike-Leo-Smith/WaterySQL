//
// Created by Mike Smith on 2018/11/4.
//

#ifndef WATERYSQL_RECORD_MANAGER_H
#define WATERYSQL_RECORD_MANAGER_H

#include <string>
#include <optional>
#include "../filesystem_demo/fileio/FileManager.h"
#include "../filesystem_demo/bufmanager/BufferedPageManager.h"
#include "record.h"
#include "table.h"

namespace watery {

class RecordManager {

public:
    template<typename Filter>
    class FilteredRecordIterator {
    private:
        RecordManager &_record_manager;
        Table &_table;
        int32_t _slot;
        Filter &&_filter;
        Record _current_record;
        uint32_t _passed_record_count;
        
        void _move_to_next_record() {
            while (_passed_record_count < _table.record_count()) {
                if (auto record = _record_manager.get_record(_table, _slot)) {
                    _passed_record_count++;
                    if (_filter(record)) {
                        _current_record = std::move(record);
                        return;
                    }
                    _slot++;
                }
            }
            _slot = -1;
        }
    
    public:
        FilteredRecordIterator(RecordManager &record_manager, Table &table, Filter &&filter)
            : _record_manager{record_manager}, _table{table}, _slot{0},
              _filter{std::forward(filter)}, _passed_record_count{0} {
            _move_to_next_record();
        }
        
        FilteredRecordIterator begin() { return *this; }
        
        FilteredRecordIterator end() {
            auto it = FilteredRecordIterator{this};
            it._slot = -1;
            return it;
        }
        
        FilteredRecordIterator &operator++() {
            _slot++;
            _move_to_next_record();
            return *this;
        }
        
        const FilteredRecordIterator operator++(int _) {
            auto it = *this;
            ++*this;
            return it;
        }
        
        bool operator!=(const FilteredRecordIterator &rhs) const { return _slot != rhs._slot; }
        Record operator*() { return _current_record; }
        
    };

private:
    FileManager _file_manager{};
    BufferedPageManager _page_manager{&_file_manager};
    
    static int32_t _record_offset(int32_t slot, uint32_t slots_per_page, uint32_t record_length);
    static uint32_t _slot_bitset_offset(uint32_t slots_per_page, int32_t slot);
    static uint8_t _slot_bitset_switcher(uint32_t slots_per_page, int32_t slot);
    
    static void _encode_record(uint8_t *buffer, const Record &record);
    static Record _decode_record(const uint8_t *buffer, const RecordDescriptor &record_descriptor);

public:
    void create_table(const std::string &name, const RecordDescriptor &record_descriptor);
    std::optional<Table> open_table(const std::string &name);
    void close_table(const Table &table);
    void delete_table(const std::string &name);
    
    Record insert_record(Table &table,
                         const RecordDescriptor &descriptor,
                         std::array<std::unique_ptr<Data>, MAX_FIELD_COUNT> fields);
    void update_record(Table &table, const Record &record);
    void delete_record(Table &table, int32_t slot);
    std::optional<Record> get_record(Table &table, int32_t slot);
    
    template<typename Filter>
    auto filter_records(Table &table, Filter &&filter) {
        return FilteredRecordIterator<Filter>{*this, table, std::forward(filter)};
    }
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
