//
// Created by Mike Smith on 2018-12-23.
//

#include "query_engine.h"
#include "../error/query_engine_error.h"
#include "../utility/memory/value_decoder.h"

namespace watery {

const Byte *QueryEngine::_make_record(
    const RecordDescriptor &desc, const Byte *raw, const uint16_t *sizes, uint16_t count) {
    
    static std::vector<Byte> record_buffer;
    
    if (count > desc.field_count) {
        throw QueryEngineError{"Too many fields to insert."};
    }
    
    if (record_buffer.size() < desc.length) {
        record_buffer.resize(desc.length);
    }
    
    auto read_ptr = raw;
    auto write_ptr = record_buffer.data();
    
    if (desc.null_mapped) {
        MemoryMapper::map_memory<NullFieldBitmap>(record_buffer.data()).set();
        write_ptr += sizeof(NullFieldBitmap);
    }
    
    if (desc.reference_counted) {
        MemoryMapper::map_memory<uint32_t>(write_ptr) = 0;
        write_ptr += sizeof(uint32_t);
    }
    
    for (auto i = 0; i < count; i++) {
        auto size = sizes[i];
        if (size == 0) {
            if (desc.field_descriptors[i].constraints.nullable()) {
                MemoryMapper::map_memory<NullFieldBitmap>(record_buffer.data())[i] = false;
            } else {
                throw QueryEngineError{"Cannot assign NULL to a column that is not nullable."};
            }
        } else {
            std::string_view raw_field{read_ptr, size};
            switch (desc.field_descriptors[i].data_descriptor.type) {
                case TypeTag::INTEGER: {
                    auto value = ValueDecoder::decode_integer(raw_field);
                    MemoryMapper::map_memory<int32_t>(write_ptr) = value;
                    break;
                }
                case TypeTag::FLOAT: {
                    auto value = ValueDecoder::decode_float(raw_field);
                    MemoryMapper::map_memory<float>(write_ptr) = value;
                    break;
                }
                case TypeTag::CHAR: {
                    auto value = ValueDecoder::decode_char(raw_field);
                    StringViewCopier::copy(value, write_ptr);
                    break;
                }
                case TypeTag::DATE: {
                    auto value = ValueDecoder::decode_date(raw_field);
                    MemoryMapper::map_memory<int32_t>(write_ptr) = value;
                    break;
                }
                default:
                    throw QueryEngineError{std::string{"Failed to decode field \""}.append(raw_field).append("\".")};
            }
        }
        read_ptr += size;
        write_ptr += desc.field_descriptors[i].data_descriptor.length;
    }
    
    return record_buffer.data();
}

void QueryEngine::insert_records(
    const std::string &table_name, const std::vector<Byte> &raw,
    const std::vector<uint16_t> &field_sizes, const std::vector<uint16_t> &field_counts) {
    
}

void QueryEngine::_insert_record(
    const std::string &table_name, const Byte *raw,
    const uint16_t *sizes, uint16_t count) {
    
    auto table = _record_manager.open_table(table_name).lock();
    auto record = _make_record(table->header.record_descriptor, raw, sizes, count);
    
    
    
}

}
