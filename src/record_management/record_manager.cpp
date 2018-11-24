//
// Created by Mike Smith on 2018/11/4.
//

#include <filesystem>

#include "record_manager.h"
#include "../utility/io_helpers/binary_stream.h"
#include "../errors/file_io_error.h"
#include "../utility/io_helpers/error_printer.h"

namespace watery {

void RecordManager::create_table(const std::string &name, const RecordDescriptor &record_descriptor) {
    
    auto file_name = name + ".table";
    if (_page_manager.file_exists(file_name)) {
        throw FileIOError{
            std::string{"Failed to create file for table \""}.append(name).append("\" which already exists.")};
    }
    
    FileHandle file_handle;
    try {
        _page_manager.create_file(file_name);
        file_handle = _page_manager.open_file(file_name);
    } catch (const FileIOError &e) {
        print_error(std::cerr, e);
        throw FileIOError{std::string{"Failed to create file for table \""}.append(name).append("\".")};
    }
    
    auto page = _page_manager.allocate_page(file_handle, 0);
    BinaryStream{page.buffer} << uint32_t{0}  // page count
                              << uint32_t{0}  // record count
                              << uint32_t{0}  // current rid
                              << record_descriptor;
    _page_manager.mark_page_dirty(page);
    _page_manager.flush_page(page);
    _page_manager.close_file(file_handle);
    
}

Table RecordManager::open_table(const std::string &name) {
    
    auto file_name = name + ".table";
    
    FileHandle file_handle = 0;
    try {
        file_handle = _page_manager.open_file(file_name);
    } catch (const FileIOError &e) {
        print_error(std::cerr, e);
        throw FileIOError(std::string{"Failed to open file for table \""}.append(name).append("\"."));
    }
    
    auto page = _page_manager.get_page(file_handle, 0);
    _page_manager.mark_page_accessed(page);
    
    BinaryStream binary_stream{page.buffer};
    
    auto page_count = binary_stream.get<uint32_t>();
    auto record_count = binary_stream.get<uint32_t>();
    auto curr_rid = binary_stream.get<int32_t>();
    auto record_descriptor = binary_stream.get<RecordDescriptor>();
    Table table{name, record_descriptor, file_handle, curr_rid, page_count, record_count};
    table.add_page_handle(page.page_handle);
    
    return table;
}

void RecordManager::close_table(const Table &table) {
    
    auto page = _page_manager.get_page(table.file_handle(), 0);
    _page_manager.mark_page_accessed(page);
    
    BinaryStream{page.buffer} << table.page_count() << table.record_count() << table.current_record_id();
    
    _page_manager.mark_page_dirty(page);
    _page_manager.flush_page(page);
    
    for (auto &&handle : table.page_handles()) {
        if (!_page_manager.is_replaced(page)) {
            _page_manager.flush_page(page);
        }
    }
    _page_manager.close_file(table.file_handle());
}

void RecordManager::delete_table(const std::string &name) {
    try {
        _page_manager.delete_file(name + ".table");
    } catch (const FileIOError &e) {
        print_error(std::cerr, e);
        throw FileIOError{std::string{"Failed to delete file for table \""}.append(name).append("\".")};
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
    
    for (auto i = 0; i < table.page_count(); i++) {
        
        auto buffer_id = 0;
        buffer = _buffer_manager.get_page(table.file_handle(), i, buffer_id);
        _buffer_manager.mark_access(buffer_id);
        table.add_page_handle(buffer_id);
        
        auto binary_stream = BinaryStream{buffer};
        if (binary_stream.get<uint32_t>() == table.slot_count_per_page()) {
            slot += table.slot_count_per_page();
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
        _buffer_manager.mark_dirty(buffer_id);
        std::cout << "Found available slot: " << i << ", " << slot << std::endl;
        break;
    }
    
    if (buffer == nullptr) {  // no available page found, alloc new
        auto buffer_id = 0;
        buffer = _buffer_manager.allocate_page(table.file_handle(), table.page_count(), buffer_id);
        table.increase_page_count();
        table.add_page_handle(buffer_id);
        _buffer_manager.mark_dirty(buffer_id);
        
        // initialize the page
        auto binary_stream = BinaryStream{buffer};
        binary_stream << uint32_t{0};
        for (int i = 0; i < table.slot_count_per_page(); i++) {
            binary_stream << uint8_t{0x00};
        }
    }
    
    auto istream = BinaryStream{buffer};
    auto ostream = BinaryStream{buffer};
    
    // update record count in page
    ostream << static_cast<uint32_t>(istream.get<uint32_t>() + 1);
    
    // update slot bitset
    auto byte_offset = _slot_bitset_offset(table.slot_count_per_page(), slot);
    istream.seek(byte_offset);
    ostream.seek(byte_offset);
    ostream << (istream.get<uint8_t>() | _slot_bitset_switcher(table.slot_count_per_page(), slot));
    
    // create new record
    auto record = Record{table.record_descriptor().field_count, table.current_record_id(), slot};
    table.increase_current_record_id();
    table.increase_record_count();
    
    auto offset = _record_offset(slot, table.slot_count_per_page(), table.record_length());
    _encode_record(reinterpret_cast<uint8_t *>(buffer) + offset, record);
    
    return record;
}

void RecordManager::update_record(Table &table, const Record &record) {
    auto index = 0;
    auto buffer = _buffer_manager.get_page(table.file_handle(), record.slot() / table.slot_count_per_page(), index);
    _buffer_manager.mark_dirty(index);
    table.add_page_handle(index);
    
    auto offset = _record_offset(record.slot(), table.slot_count_per_page(), table.record_length());
    _encode_record(reinterpret_cast<uint8_t *>(buffer) + offset, record);
}

void RecordManager::delete_record(Table &table, int32_t slot) {
    auto index = 0;
    auto buffer = _buffer_manager.get_page(table.file_handle(), slot / table.slot_count_per_page(), index);
    _buffer_manager.mark_dirty(index);
    table.add_page_handle(index);
    
    auto istream = BinaryStream{buffer};
    auto ostream = BinaryStream{buffer};
    
    ostream << static_cast<uint32_t>(istream.get<uint32_t>() - 1);
    auto byte_pos = _slot_bitset_offset(table.slot_count_per_page(), slot);
    
    istream.seek(byte_pos);
    ostream.seek(byte_pos);
    ostream
        << static_cast<uint8_t>((istream.get<uint8_t>() & ~_slot_bitset_switcher(table.slot_count_per_page(), slot)));
    
    table.decrease_record_count();
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

std::optional<Record> RecordManager::get_record(Table &table, int32_t slot) {
    
    auto index = 0;
    auto buffer = _buffer_manager.get_page(table.file_handle(), slot / table.slot_count_per_page(), index);
    _buffer_manager.mark_access(index);
    table.add_page_handle(index);
    
    auto byte_pos = _slot_bitset_offset(table.slot_count_per_page(), slot);
    auto bit_mask = _slot_bitset_switcher(table.slot_count_per_page(), slot);
    if ((reinterpret_cast<uint8_t *>(buffer)[byte_pos] & bit_mask) == 0) {
        std::cerr << "Failed to get record: record does not exist or is removed." << std::endl;
        return std::nullopt;
    }
    
    auto offset = _record_offset(slot, table.slot_count_per_page(), table.record_length());
    auto record = _decode_record(reinterpret_cast<uint8_t *>(buffer) + offset, table.record_descriptor());
    record.set_slot(slot);
    
    return record;
}

}
