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
    
    Record insert_record(Table &table, const RecordDescriptor &descriptor, std::array<std::unique_ptr<Data>, MAX_FIELD_COUNT> fields);
    void update_record(Table &table, const Record &record);
    void delete_record(Table &table, int32_t slot);
    Record get_record(Table &table, int32_t slot);
    
    template<typename Filter>
    std::vector<Record> filter_records(Table &table, Filter &&filter) {
        
    }
    
};

}

#endif  // WATERYSQL_RECORD_MANAGER_H
