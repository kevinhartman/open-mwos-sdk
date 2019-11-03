#include "ROFWriter.h"

#include "ROFObjectFile.h"

namespace rof {

ROFWriter::ROFWriter(uint16_t compiler_version) : compiler_version(compiler_version) {}

void WriteHeader(const ROFObjectFile& rof_object) {

}

void ROFWriter::Write(const ROFObjectFile& rof_object) {

}

}