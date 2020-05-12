#pragma once

#include <Endian.h>

#include <ostream>

namespace object {
    class ObjectFile;
}

namespace rof {

class ROFObjectWriter {
public:
    ROFObjectWriter(support::Endian endianness);
    void Write(const object::ObjectFile& object_file, std::ostream& out) const;

private:
    support::Endian endianness;
};

}