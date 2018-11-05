//
// Created by Mike Smith on 2018/11/4.
//

#include <filesystem>

#include "record_manager.h"
#include "../utility/binary_stream.h"

namespace watery {

void RecordManager::create_table(const std::string &name, const RecordDescriptor &record_descriptor) {
    
    auto file_name = name + ".table";
    if (std::filesystem::exists(file_name)) {
        std::cerr << "Failed to create table \"" << name << "\": file with the same name already exists." << std::endl;
        return;
    }
    
    if (!_file_manager.createFile(file_name.c_str())) {
        std::cerr << "Failed to create table \"" << name << "\": file cannot be created." << std::endl;
        return;
    }
    
    auto file_id = 0;
    if (!_file_manager.openFile(file_name.c_str(), file_id)) {
        std::cerr << "Failed to create table \"" << name << "\": file cannot be opened." << std::endl;
        return;
    }
    
    auto buffer_id = 0;
    
    // write header to the first page.
    auto binary_stream = BinaryStream{_page_manager.allocPage(file_id, 0, buffer_id)};
    binary_stream << uint32_t{0}  // page count
                  << uint32_t{0}  // record count
                  << uint32_t{0}  // current rid
                  << record_descriptor;
    _page_manager.markDirty(buffer_id);
    _page_manager.writeBack(buffer_id);
    _file_manager.closeFile(file_id);
}

std::optional<Table> RecordManager::open_table(const std::string &name) {
    
    auto file_name = name + ".table";
    auto file_id = 0;
    if (!_file_manager.openFile(file_name.c_str(), file_id)) {
        std::cout << "Failed to open table \"" << name << "\": file cannot be opened." << std::endl;
        return std::nullopt;
    }
    
    auto buffer_id = 0;
    auto binary_stream = BinaryStream{_page_manager.getPage(file_id, 0, buffer_id)};
    _page_manager.markAccess(buffer_id);
    
    auto page_count = binary_stream.get<uint32_t>();
    auto record_count = binary_stream.get<uint32_t>();
    auto curr_rid = binary_stream.get<int32_t>();
    auto record_descriptor = binary_stream.get<RecordDescriptor>();
    auto table = Table{name, record_descriptor, file_id, curr_rid, page_count, record_count};
    table.buffer_ids.emplace(buffer_id);
    
    return table;
}

void RecordManager::close_table(const Table &table) {
    
    auto buffer_id = 0;
    auto binary_stream = BinaryStream{_page_manager.getPage(table.file_id, 0, buffer_id)};
    _page_manager.markAccess(buffer_id);
    
    binary_stream << table.page_count << table.record_count << table.current_rid;
    _page_manager.markDirty(buffer_id);
    _page_manager.writeBack(buffer_id);
    
    for (auto &&id : table.buffer_ids) {
        auto file_id = 0;
        auto page_id = 0;
        _page_manager.getKey(id, file_id, page_id);
        if (file_id == table.file_id) {
            _page_manager.writeBack(id);
        }
    }
    _file_manager.closeFile(table.file_id);
}

void RecordManager::delete_table(const std::string &name) {
    if (!std::filesystem::remove(name + ".table")) {
        std::cerr << "Failed to delete table \"" << name << "\": file does not exist or cannot be remove." << std::endl;
    }
}

Record RecordManager::_decode_record(const uint8_t *buffer, const RecordDescriptor &record_descriptor) {
    auto &&record = Record{record_descriptor.field_count, *reinterpret_cast<const int32_t *>(buffer)};
    auto ptr = buffer + sizeof(uint32_t);
    for (auto i = 0; i < record_descriptor.field_count; i++) {
        auto &&data_descriptor = record_descriptor.field_descriptors[i].data_descriptor;
        record.set_field(i, Data::decode(data_descriptor, ptr));
        ptr += data_descriptor.size;
    }
    return std::move(record);
}

void RecordManager::_encode_record(uint8_t *buffer, const Record &record) {
    *reinterpret_cast<int32_t *>(buffer) = record.id();
    buffer += sizeof(uint32_t);
    for (auto &&data: record.fields()) {
        data->encode(buffer);
        buffer += data->descriptor().size;
    }
}

Record RecordManager::insert_record(
    Table &table, const RecordDescriptor &descriptor,
    std::array<std::unique_ptr<Data>, MAX_FIELD_COUNT> fields) {
    
    auto slot = 0;
    auto buffer = static_cast<BufType>(nullptr);
    
    for (auto i = 0; i < table.page_count; i++) {
    
        auto buffer_id = 0;
        buffer = _page_manager.getPage(table.file_id, i, buffer_id);
        _page_manager.markAccess(buffer_id);
        table.buffer_ids.emplace(buffer_id);
        
        auto binary_stream = BinaryStream{buffer};
        if (binary_stream.get<uint32_t>() == table.slot_count_per_page) {
            slot += table.slot_count_per_page;
            continue;
        }
        
        auto byte = 0;
        while ((byte = binary_stream.get<uint8_t>()) == 0xff) {
            slot += 8;
        }
        while ((byte & 1) == 1) {
            slot++;
            byte >>= 1;
        }
        _page_manager.markDirty(buffer_id);
        std::cout << "Found available slot: " << i << ", " << slot << std::endl;
        break;
    }
    
    if (buffer == nullptr) {  // no available page found, alloc new
        auto buffer_id = 0;
        buffer = _page_manager.allocPage(table.file_id, table.page_count, buffer_id);
        table.page_count++;
        table.buffer_ids.emplace(buffer_id);
        _page_manager.markDirty(buffer_id);
        
        // initialize the page
        auto binary_stream = BinaryStream{buffer};
        table.buffer_ids.emplace(buffer_id);
        binary_stream << uint32_t{0};
        for (int i = 0; i < table.slot_count_per_page; i++) {
            binary_stream << uint8_t{0x00};
        }
    }
    
    auto istream = BinaryStream{buffer};
    auto ostream = BinaryStream{buffer};
    
    // update record count in page
    ostream << static_cast<uint32_t>(istream.get<uint32_t>() + 1);
    
    // update slot bitset
    auto byte_offset = _slot_bitset_offset(table.slot_count_per_page, slot);
    istream.seek(byte_offset);
    ostream.seek(byte_offset);
    ostream << (istream.get<uint8_t>() | _slot_bitset_switcher(table.slot_count_per_page, slot));
    
    // create new record
    auto record = Record{table.record_descriptor.field_count, table.current_rid, slot};
    table.current_rid++;
    table.record_count++;
    
    auto offset = _record_offset(slot, table.slot_count_per_page, table.record_length);
    _encode_record(reinterpret_cast<uint8_t *>(buffer) + offset, record);
    
    return record;
}

void RecordManager::update_record(Table &table, const Record &record) {
    auto index = 0;
    auto buffer = _page_manager.getPage(table.file_id, record.slot() / table.slot_count_per_page, index);
    table.buffer_ids.emplace(index);
    _page_manager.markDirty(index);
    
    auto offset = _record_offset(record.slot(), table.slot_count_per_page, table.record_length);
    _encode_record(reinterpret_cast<uint8_t *>(buffer) + offset, record);
}

void RecordManager::delete_record(Table &table, int32_t slot) {
    auto index = 0;
    auto buffer = _page_manager.getPage(table.file_id, slot / table.slot_count_per_page, index);
    _page_manager.markDirty(index);
    table.buffer_ids.emplace(index);
    
    auto istream = BinaryStream{buffer};
    auto ostream = BinaryStream{buffer};
    
    ostream << static_cast<uint32_t>(istream.get<uint32_t>() - 1);
    auto byte_pos = _slot_bitset_offset(table.slot_count_per_page, slot);
    
    istream.seek(byte_pos);
    ostream.seek(byte_pos);
    ostream << static_cast<uint8_t>((istream.get<uint8_t>() & ~_slot_bitset_switcher(table.slot_count_per_page, slot)));
    
    table.record_count--;
}

uint8_t RecordManager::_slot_bitset_switcher(uint32_t slots_per_page, int32_t slot) {
    return uint8_t{1} << (slot % slots_per_page % 8);
}

uint32_t RecordManager::_slot_bitset_offset(uint32_t slots_per_page, int32_t slot) {
    return sizeof(uint32_t) + slot % slots_per_page / 8;
}

int32_t RecordManager::_record_offset(int32_t slot, uint32_t slots_per_page, uint32_t record_length) {
    return sizeof(uint32_t) + SLOT_BITSET_SIZE + slot % slots_per_page * record_length;
}

Record RecordManager::get_record(Table &table, int32_t slot) {
    
    auto index = 0;
    auto buffer = _page_manager.getPage(table.file_id, slot / table.slot_count_per_page, index);
    _page_manager.markAccess(index);
    table.buffer_ids.emplace(index);
    
    auto offset = _record_offset(slot, table.slot_count_per_page, table.record_length);
    auto record = _decode_record(reinterpret_cast<uint8_t *>(buffer) + offset, table.record_descriptor);
    record.set_slot(slot);
    
    return record;
}

}
