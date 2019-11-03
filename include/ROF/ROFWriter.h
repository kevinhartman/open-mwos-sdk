#pragma once

#include <cstdint>

namespace rof {

class ROFObjectFile;

class ROFWriter {
public:
    ROFWriter(uint16_t compiler_version);

    void Write(const ROFObjectFile&);

private:
    uint16_t compiler_version;

};

}