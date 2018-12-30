//
// Created by Mike Smith on 2018-12-15.
//

#ifndef WATERYSQL_DATA_VIEW_H
#define WATERYSQL_DATA_VIEW_H

#include <variant>

#include "type_tag.h"
#include "../config/config.h"
#include "data_descriptor.h"

#include "../utility/memory/memory_mapper.h"
#include "../error/data_error.h"
#include "../utility/memory/string_view_copier.h"
#include "../utility/memory/value_decoder.h"

namespace watery {

struct DataView {
    
    DataDescriptor descriptor;
    std::variant<int32_t, float, std::string_view> holder;
    
    DataView(DataDescriptor desc, const Byte *buf) : descriptor{desc} {
        switch (descriptor.type) {
            case TypeTag::INTEGER:
            case TypeTag::DATE:
                holder = MemoryMapper::map_memory<int32_t>(buf);
                break;
            case TypeTag::FLOAT:
                holder = MemoryMapper::map_memory<float>(buf);
                break;
            case TypeTag::CHAR:
                holder = std::string_view{buf};
                break;
            default:
                throw DataError{"Unknown type tag."};
        }
    }

//    DataView(DataDescriptor desc, std::string_view raw) : descriptor{desc} {
//        switch (descriptor.type) {
//            case TypeTag::INTEGER:
//            case TypeTag::DATE:
//                holder = ValueDecoder::decode_integer(raw);
//                break;
//            case TypeTag::FLOAT:
//                holder = ValueDecoder::decode_float(raw);
//                break;
//            case TypeTag::CHAR:
//                holder = raw;
//                break;
//            default:
//                throw DataError{"Unknown type tag."};
//        }
//    }
    
    std::string to_string() const {
        switch (descriptor.type) {
            case TypeTag::DATE: {
                std::string result = "0000-00-00";
                auto date = std::get<int32_t>(holder);
                auto year = date >> 16;
                auto month = (date >> 8) & 0xff;
                auto day = date & 0xff;
                result[0] = static_cast<char>(year / 1000 + '0');
                result[1] = static_cast<char>(year / 100 % 10 + '0');
                result[2] = static_cast<char>(year / 10 % 10 + '0');
                result[3] = static_cast<char>(year % 10 + '0');
                result[5] = static_cast<char>(month / 10 + '0');
                result[6] = static_cast<char>(month % 10 + '0');
                result[8] = static_cast<char>(day / 10 + '0');
                result[9] = static_cast<char>(day % 10 + '0');
                return result;
            }
            case TypeTag::INTEGER:
                return std::to_string(std::get<int32_t>(holder));
            case TypeTag::FLOAT:
                return std::to_string(std::get<float>(holder));
            case TypeTag::CHAR:
                return std::string{std::get<std::string_view>(holder)};
            default:
                throw DataError{"Unknown type tag."};
        }
    }
    
};

}

#endif  // WATERYSQL_DATA_VIEW_H
