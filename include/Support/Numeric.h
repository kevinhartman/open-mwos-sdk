#pragma once

#include <limits>

namespace support {

template <typename T>
struct MaxRangeOf {
    static constexpr auto value = std::numeric_limits<std::decay_t<T>>::max();
};

}