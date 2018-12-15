//
// Created by Mike Smith on 2018/11/5.
//

#ifndef WATERYSQL_BINARY_STREAM_H
#define WATERYSQL_BINARY_STREAM_H

#include <cstdint>
#include <memory>
#include "../type/non_copyable.h"

namespace watery {

class BinaryStream {
private:
    uint8_t *_buffer;
    uint32_t _offset;

public:
    
    template<typename T>
    explicit BinaryStream(T *buffer) : _buffer{reinterpret_cast<uint8_t *>(buffer)}, _offset{0} {}
    
    template<typename T>
    void write(const T *object, uint32_t size = sizeof(T)) {
        std::uninitialized_copy_n(reinterpret_cast<const uint8_t *>(object), size, _buffer + _offset);
        _offset += size;
    }
    
    template<typename T>
    void read(T *object, uint32_t size = sizeof(T)) {
        std::uninitialized_copy_n(_buffer + _offset, size, reinterpret_cast<uint8_t *>(object));
        _offset += size;
    }
    
    template<typename T>
    T &get() {
        auto &&object = *reinterpret_cast<T *>(_buffer + _offset);
        _offset += sizeof(T);
        return object;
    }
    
    template<typename T>
    const T &get() const {
        return get<T>();
    }
    
    template<typename T>
    T *as() {
        return reinterpret_cast<T *>(_buffer);
    }
    
    template<typename T>
    const T *as() const {
        return as<T>();
    }
    
    void move(int32_t delta_offset) {
        _offset += delta_offset;
    }
    
    void seek(uint32_t offset = 0) {
        _offset += offset;
    }
    
    template<typename T>
    BinaryStream &operator<<(T &&object) {
        write(&object);
        return *this;
    }
    
    template<typename T>
    BinaryStream &operator>>(T &&object) {
        read(&object);
        return *this;
    }
    
};

}

#endif  // WATERYSQL_BINARY_STREAM_H
