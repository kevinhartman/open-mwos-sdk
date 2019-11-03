#pragma once

// TODO: there might be an issue here since this means we need
// to ensure CMake always includes the support headers whenever
// it includes ROF headers.
#include "Endian.h"
//

#include <string>

namespace rof {
class ROFHeader;
}

namespace rof::util {
void SerializeHeader(const ROFHeader& rof_header, support::Endian endianness, char *buffer);
}
