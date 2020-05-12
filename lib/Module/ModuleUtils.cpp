// Copyright 2016 Kevin Hartman
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of the owner nor the names of its contributors may
//   be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "BinarySectionReader.hpp"
#include "ModuleUtils.hpp"
#include "CrcGenerator.hpp"

#include <cstring>
#include <type_traits>

namespace module::util {

using BinarySectionReader = support::BinarySectionReader;

bool IsBigEndian(const char* header_data) {
    uint8_t mips[2] = { 0x4D, 0xAD };
    uint8_t _68k[2] = { 0x4A, 0xFC };
    uint8_t i386[2] = { 0xFC, 0x4A };

    if (memcmp(header_data, mips, 2) == 0 ||
        memcmp(header_data, _68k, 2) == 0)
        return true;

    if (memcmp(header_data, i386, 2) == 0) return false;

    assert(false); // Unknown OS-9 Port / Arch
    return false;
}

support::Endian EndianOf(const char* header_data) {
    return IsBigEndian(header_data) ? support::Endian::big : support::Endian::little;
}

//void ParseDataReferenceList(uint32_t** ref_list, const char* module_data, const char* table_data)
//{
//    BinarySectionReader ref_section(table_data);
//
//    uint16_t msw_offset;
//    ref_section.ReadNext(&msw_offset);
//
//    uint16_t lsw_count;
//    ref_section.ReadNext(&lsw_count);
//
//    BinarySectionReader module_section(module_data);
//
//    module_section.Seek(msw_offset);
//
//}

uint16_t CalculateHeaderParity(const char* header_data) {
    static_assert(sizeof(ModuleHeader) % 2 == 0, "Parity calculation assumes header is comprised of whole words.");

    BinarySectionReader parser(header_data,
        sizeof(ModuleHeader) - sizeof(std::remove_reference<decltype(std::declval<ModuleHeader>().Parity())>::type),
        EndianOf(header_data));

    uint16_t parity = 0;
    uint16_t word;
    while (parser.ReadNext(&word)) {
        parity ^= word;
    }

    return ~parity;
}

uint32_t CalculateCrcComplement(const char* module_data, size_t size) {
    uint32_t crc = -1;
    module::crc::Generate(module_data, size, &crc);

    // Add one zero byte to CRC to account for first unused byte of CRC 32-bit field
    const char zero[] = { 0 };
    module::crc::Generate(zero, 1, &crc);

    return ~crc;
}

}