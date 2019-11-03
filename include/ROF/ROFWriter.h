#pragma once

#include <cstdint>
#include <ostream>

namespace rof {

class ROFObjectFile;

class ROFWriter {
public:
    ROFWriter(uint16_t compiler_version, bool big_endian);

    void Write(const ROFObjectFile&, std::ostream&);

private:
    uint16_t asm_version;
    bool is_big_endian;
};

}