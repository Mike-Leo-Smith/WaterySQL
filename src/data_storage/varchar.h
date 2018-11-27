//
// Created by Mike Smith on 2018/11/25.
//

#ifndef WATERYSQL_VARCHAR_H
#define WATERYSQL_VARCHAR_H

#include <vector>
#include "data.h"

namespace watery {

class Varchar : public Data {
private:
    std::string _val;

public:
    Varchar(const char *buffer, uint32_t size)
        : _val{buffer, size} {}
    
    TypeTag type() const override { return TypeTag::VARCHAR; }
    uint32_t length() const override { return static_cast<uint32_t>(_val.size()); }
    const std::string &value() const { return _val; }
    
    void encode(Byte *buffer) const override {
        std::memmove(buffer, _val.c_str(), _val.size());
    }
    
    std::unique_ptr<Data> replica() const override {
        return std::make_unique<Varchar>(_val.data(), _val.size());
    }
    
    bool operator<(const Data &rhs) const override {
        return rhs.type() == TypeTag::VARCHAR ?
               (_val < reinterpret_cast<const Varchar &>(rhs)._val) :
               Data::operator<(rhs);
    }
    
    bool operator<=(const Data &rhs) const override {
        return rhs.type() == TypeTag::VARCHAR ?
               (_val <= reinterpret_cast<const Varchar &>(rhs)._val) :
               Data::operator<=(rhs);
    }
    
    bool operator==(const Data &rhs) const override {
        return rhs.type() == TypeTag::VARCHAR ?
               (_val == reinterpret_cast<const Varchar &>(rhs)._val) :
               Data::operator==(rhs);
    }
    
    bool operator>=(const Data &rhs) const override {
        return rhs.type() == TypeTag::VARCHAR ?
               (_val >= reinterpret_cast<const Varchar &>(rhs)._val) :
               Data::operator>=(rhs);
    }
    
    bool operator>(const Data &rhs) const override {
        return rhs.type() == TypeTag::VARCHAR ?
               (_val > reinterpret_cast<const Varchar &>(rhs)._val) :
               Data::operator>(rhs);
    }
    
};

}

#endif  // WATERYSQL_VARCHAR_H
