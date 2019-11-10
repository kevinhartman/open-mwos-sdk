#pragma once

#include "Endian.h"

#include <cstdint>
#include <ostream>

namespace rof {

class ROFObjectFile;

class ROFWriter {
public:
    ROFWriter(uint16_t compiler_version, support::Endian endianness);

    void Write(const ROFObjectFile&, std::ostream&);

private:
    uint16_t asm_version;
    support::Endian endianness;
};

}