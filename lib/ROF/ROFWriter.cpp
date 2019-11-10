#include "Endian.h"
#include "ROFObjectFile.h"
#include "TupleSerialization.h"

#include "ROFWriter.h"

namespace rof {

ROFWriter::ROFWriter(uint16_t compiler_version, support::Endian endianness): asm_version(compiler_version), endianness(endianness) {}

void ROFWriter::Write(const ROFObjectFile& rof_object, std::ostream& file) {
    // Write header to file
    serializer::SerializeTuple(static_cast<SerializableROFHeader>(rof_object.GetHeader()), file, endianness);

    // Write ROF name to file.
    auto name = rof_object.GetName();
    file.write(name.c_str(), name.length() + 1 /* for terminator */);

    // Write external definition section


}

}