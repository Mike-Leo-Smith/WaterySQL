//
// Created by Mike Smith on 2018-12-23.
//

#ifndef WATERYSQL_QUERY_ENGINE_H
#define WATERYSQL_QUERY_ENGINE_H

#include <memory>

#include "../config/config.h"
#include "../record/table.h"
#include "../index/index_manager.h"
#include "../record/record_manager.h"

namespace watery {

class QueryEngine {

private:
    RecordManager &_record_manager{RecordManager::instance()};
    IndexManager &_index_manager{IndexManager::instance()};
    
    const Byte *_make_record(const RecordDescriptor &desc, const Byte *raw, const uint16_t *sizes, uint16_t count);
    void _insert_record(const std::string &table_name, const Byte *raw, const uint16_t *sizes, uint16_t count);
    
public:
    void insert_records(
        const std::string &table_name, const std::vector<Byte> &raw,
        const std::vector<uint16_t> &field_sizes, const std::vector<uint16_t> &field_counts);

};

}

#endif  // WATERYSQL_QUERY_ENGINE_H
