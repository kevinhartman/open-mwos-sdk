#include <TupleSerialization.h>
#include "ROFObjectFile.h"

namespace rof {

std::vector<ExternDefinition> ROFObjectFile::GetExternalDefinitions() const {
    // TODO: iterate over PSect and VSects to gather external data references.
    // Note that order should be considered (remote vs local, etc.)
}

void ROFObjectFile::Write(std::ostream& out_stream, support::Endian endianness) {
    // Write header to file
    serializer::SerializeTuple(static_cast<SerializableROFHeader>(GetHeader()), out_stream, endianness);

    // Write external definition section
}

}