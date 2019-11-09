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

#pragma once

#include "SerializableStruct.h"

#include <array>
#include <cstdint>

namespace module {

struct ModuleHeader : SerializableStruct<
    SequenceOfType<uint16_t, 2>,
    SequenceOfType<uint32_t, 3>,
    SequenceOfType<uint16_t, 4>,
    SequenceOfType<uint32_t, 13>,
    uint16_t,
    PadBytes<8>,
    uint16_t
> {
    auto& SyncBytes() { return std::get<0>(*this); }
    auto& SystemRevision() { return std::get<1>(*this); }

    auto& Size() { return std::get<2>(*this); }
    auto& Owner() { return std::get<3>(*this); }
    auto& OffsetToName() { return std::get<4>(*this); }

    auto& Access() { return std::get<5>(*this); }
    auto& TypeLanguage() { return std::get<6>(*this); }
    auto& AttRev() { return std::get<7>(*this); }
    auto& Edition() { return std::get<8>(*this); }

    auto& HardwareNeeds() { return std::get<9>(*this); }
    auto& OffsetToShared() { return std::get<10>(*this); }
    auto& OffsetToSymbol() { return std::get<11>(*this); }
    auto& OffsetToExec() { return std::get<12>(*this); }
    auto& OffsetToExcept() { return std::get<13>(*this); }
    auto& SizeOfData() { return std::get<14>(*this); }
    auto& MinStackSize() { return std::get<15>(*this); }
    auto& InitializedDataOffset() { return std::get<16>(*this); }
    auto& InitDataRefOffset() { return std::get<17>(*this); }
    auto& InitOffset() { return std::get<18>(*this); }
    auto& TermOffset() { return std::get<19>(*this); }
    auto& DataBias() { return std::get<20>(*this); }
    auto& CodeBias() { return std::get<21>(*this); }

    auto& LinkIdent() { return std::get<22>(*this); }

    auto& Parity() { return std::get<24>(*this); }
};

struct InitDataHeader {
    uint64_t field;

    uint32_t GetOffset() {
        return field >> 32U;
    }

    uint32_t GetByteCount() {
        return field & 0xFFFFFFFF;
    }
} __attribute__((packed));

}