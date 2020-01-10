#pragma once

#include <Endian.h>

#include <ostream>

namespace rof {

class ROFObjectFile;

class ROFObjectWriter {
public:
    ROFObjectWriter(support::Endian endianness);
    void Write(const ROFObjectFile& rof_object, std::ostream& out) const;

private:
    support::Endian endianness;
};

}