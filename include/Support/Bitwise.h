#pragma once

#include <cstdint>

namespace support {

uint32_t RoundUpToNextPow2(uint32_t input) {
    input--;
    input |= input >> 1;
    input |= input >> 2;
    input |= input >> 4;
    input |= input >> 8;
    input |= input >> 16;
    input++;

    return input;
}

bool IsPow2(uint32_t input) {
    return input > 0 && !(input & (input - 1));
}

size_t RoundToNextPow2Multiple(size_t input, size_t pow_2_multiple)
{
    assert(IsPow2(pow_2_multiple));
    return (input + pow_2_multiple - 1) & -pow_2_multiple;
}

}