//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_FLOAT_H
#define WATERYSQL_FLOAT_H

#include "data.h"
#include "../utility/memory_mapping/memory_mapper.h"
#include "../errors/data_error.h"
#include "integer.h"

namespace watery {

class Float : public Data {
private:
    float _val;

public:
    explicit Float(float val) : _val{val} {}
    
    TypeTag type() const override { return TypeTag::FLOAT; }
    uint32_t length() const override { return sizeof(float); }
    float value() const { return _val; }
    
    void encode(Byte *buffer) const override {
        MemoryMapper::map_memory<float>(buffer) = _val;
    }
    
    bool operator<(const Data &rhs) const override {
        switch (rhs.type()) {
        case TypeTag::INTEGER:
            return _val < reinterpret_cast<const Integer &>(rhs).value();
        case TypeTag::FLOAT:
            return _val < reinterpret_cast<const Float &>(rhs).value();
        default:
            return Data::operator<(rhs);
        }
    }
    
    std::unique_ptr<Data> replica() const override {
        return std::make_unique<Float>(_val);
    }
    
    bool operator<=(const Data &rhs) const override {
        return false;
    }
    
    bool operator==(const Data &rhs) const override {
        return false;
    }
    
    bool operator>=(const Data &rhs) const override {
        return false;
    }
    
    bool operator>(const Data &rhs) const override {
        return false;
    }
    
};

}

#endif  // WATERYSQL_FLOAT_H
