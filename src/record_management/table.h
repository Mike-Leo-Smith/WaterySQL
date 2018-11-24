#include <utility>

//
// Created by Mike Smith on 2018/11/5.
//

#ifndef WATERYSQL_TABLE_H
#define WATERYSQL_TABLE_H

#include <string>
#include <unordered_set>
#include "../utility/type_constraints/non_copyable.h"
#include "record_descriptor.h"
#include "../filesystem_demo/utils/pagedef.h"
#include "../utility/type_constraints/non_movable.h"

namespace watery {

class Table : NonCopyable {
private:
    std::string _name;
    RecordDescriptor _record_descriptor;
    int32_t _file_id;
    uint32_t _page_count;
    uint32_t _record_count;
    uint32_t _record_length;
    uint32_t _slot_count_per_page;
    int32_t _current_rid;
    std::unordered_set<int32_t> _buffer_ids;
    
public:
    Table(std::string name, const RecordDescriptor &rd, int32_t fid = -1,
          int32_t curr_rid = 0, uint32_t pc = 0, uint32_t rc = 0);
    
    void increase_page_count();
    void increase_record_count();
    void decrease_record_count();
    void add_page_handle(int32_t id);
    void increase_current_record_id();
    
    const std::string &name() const;
    const RecordDescriptor &record_descriptor() const;
    int32_t file_handle() const;
    uint32_t page_count() const;
    uint32_t record_count() const;
    uint32_t record_length() const;
    uint32_t slot_count_per_page() const;
    int32_t current_record_id() const;
    const std::unordered_set<int32_t> &page_handles() const;
    
};

}

#endif  // WATERYSQL_TABLE_H
