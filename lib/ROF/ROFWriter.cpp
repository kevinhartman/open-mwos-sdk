#include "Endian.h"
#include "ROFObjectFile.h"
#include "ROFUtils.h"

#include "ROFWriter.h"

namespace rof {

ROFWriter::ROFWriter(uint16_t compiler_version, bool big_endian): asm_version(compiler_version), is_big_endian(big_endian) {}

void ROFWriter::Write(const ROFObjectFile& rof_object, std::ostream& file) {
    std::array<char, sizeof(ROFHeader)> rof_header_buffer {};

    // Write header to buffer.
    util::SerializeHeader(rof_object.GetHeader(),
        is_big_endian ? :: support::Endian::big : ::support::Endian::little,
        rof_header_buffer.data());

    // Write header to file.
    file.write(rof_header_buffer.data(), rof_header_buffer.size());

    // Write ROF name to file.
    auto name = rof_object.GetName();
    file.write(name.c_str(), name.length() + 1 /* for terminator */);




}

}