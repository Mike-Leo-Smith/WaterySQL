//
// Created by Mike Smith on 2018/11/5.
//

#include "table.h"

namespace watery {

Table::Table(std::string name, const RecordDescriptor &rd, int32_t fid, int32_t curr_rid, uint32_t pc, uint32_t rc)
    : _name{std::move(name)}, _record_descriptor{rd}, _file_id{fid}, _current_rid{curr_rid},
      _page_count{pc}, _record_count{rc}, _record_length{rd.length()}, _slot_count_per_page{
        (PAGE_SIZE - SLOT_BITSET_SIZE - static_cast<uint32_t>(sizeof(uint32_t))) / _record_length} {}
        
const std::string &Table::name() const {
    return _name;
}

const RecordDescriptor &Table::record_descriptor() const {
    return _record_descriptor;
}

int32_t Table::file_handle() const {
    return _file_id;
}

uint32_t Table::page_count() const {
    return _page_count;
}

uint32_t Table::record_count() const {
    return _record_count;
}

uint32_t Table::record_length() const {
    return _record_length;
}

uint32_t Table::slot_count_per_page() const {
    return _slot_count_per_page;
}

const std::unordered_set<int32_t> &Table::page_handles() const {
    return _buffer_ids;
}

void Table::increase_page_count() {
    _page_count++;
}

void Table::increase_record_count() {
    _record_count++;
}

void Table::decrease_record_count() {
    _record_count--;
}

void Table::add_page_handle(int32_t id) {
    _buffer_ids.emplace(id);
}

void Table::increase_current_record_id() {
    _current_rid++;
}

int32_t Table::current_record_id() const {
    return _current_rid;
}

}