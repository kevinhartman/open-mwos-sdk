#pragma once

#include <algorithm>

namespace support {

enum Endian {
    big,
    little,
    ignore
};

#if defined(BYTE_ORDER) && defined(BIG_ENDIAN) && BYTE_ORDER == BIG_ENDIAN
constexpr Endian HostEndian = big;
#else
constexpr Endian HostEndian = little;
#endif

template<class T>
static void EndianSwap(T* field) {
    auto* raw = reinterpret_cast<unsigned char*>(field);
    std::reverse(raw, raw + sizeof(T));
}

}
