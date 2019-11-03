#pragma once

#include "Endian.h"

#include <algorithm>
#include <cassert>

namespace support {

class BinarySectionWriter {
private:
    char* data;
    const size_t data_size = 0;
    Endian endianness;

    size_t offset = 0;

public:
    BinarySectionWriter(char* data, size_t data_size, Endian endianness) : data(data), data_size(data_size), endianness(endianness) {}

    BinarySectionWriter(char* data, Endian endianness) : data(data), endianness(endianness) {}

    inline void Seek(size_t offset) {
        this->offset = offset;
    }

    inline size_t GetOffset() {
        return this->offset;
    }

    template<class T, typename = std::enable_if<std::is_scalar<T>::value>>
    bool Write(const T& field) {
        if (data_size != 0 && offset + sizeof(T) > data_size) return false;

        T endian_correct = field;
        if (endianness != ignore && endianness != HostEndian) {
            support::EndianSwap(&endian_correct);
        }

        T* write_to = reinterpret_cast<T*>(data + offset);
        *write_to = endian_correct;

        offset += sizeof(T);

        return true;
    }

    template<int Size>
    bool Write(const std::array<char, Size> field) {
        if (data_size != 0 && offset + sizeof(field) > data_size) return false;

        for (auto character : field) {
            *(data + offset++) = character;
        }

        return true;
    }
};

}
