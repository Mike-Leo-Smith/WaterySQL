//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_INTEGER_H
#define WATERYSQL_INTEGER_H

#include "data.h"
#include "../utility/memory_mapping/memory_mapper.h"

namespace watery {

class Integer : public Data {
private:
    int32_t _val;

public:
    explicit Integer(int32_t val) : _val{val} {}
    
    TypeTag type() const override { return TypeTag::INTEGER; }
    uint32_t length() const override { return sizeof(int32_t); }
    int32_t value() const { return _val; }
    void encode(Byte *buffer) const override {
        MemoryMapper::map_memory<int32_t>(buffer) = _val;
    }
    
    std::unique_ptr<Data> replica() const override {
        return std::make_unique<Integer>(_val);
    }
    
    bool operator<(const Data &data) const override {
        return false;
    }
    
    bool operator<=(const Data &data) const override {
        return false;
    }
    bool operator==(const Data &data) const override {
        return false;
    }
    
    bool operator>=(const Data &data) const override {
        return false;
    }
    
    bool operator>(const Data &data) const override {
        return false;
    }
    
};

}

#endif  // WATERYSQL_INTEGER_H
