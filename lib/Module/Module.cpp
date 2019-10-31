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

#include <cstdint>

#include "Module.hpp"
#include "CrcGenerator.hpp"
#include "ModuleUtils.hpp"
#include "BinarySection.hpp"

Module::Module(const char* raw_module) : raw_module(raw_module)
{
    ModuleUtils::ParseHeader(this->header, raw_module);
}

std::string Module::GetName()
{
    return std::string(&this->raw_module[this->header.offset_name]);
}

bool Module::IsBigEndian()
{
    return ModuleUtils::IsBigEndian(this->raw_module);
}

bool Module::IsHeaderValid()
{
    return this->header.parity == ModuleUtils::CalculateHeaderParity(this->raw_module);
}

bool Module::IsCrcValid()
{
    const uint32_t crc_constant = 0x800fe3;

    uint32_t crc = -1;
    CrcGenerator::Generate(this->raw_module, this->header.size, &crc);

    return (crc & 0xFFFFFF) == crc_constant;
}

InitDataHeader Module::GetInitializationDataHeader()
{
    BinarySection section(this->raw_module + this->header.offset_idata, sizeof(InitDataHeader),
        ModuleUtils::IsBigEndian(this->raw_module));

    InitDataHeader initDataHeader;
    section.ReadNext(&initDataHeader);

    return initDataHeader;
}

void Module::GetDataReferenceList(uint32_t* unadjusted_pointers)
{
    BinarySection section(this->raw_module + this->header.offset_idref, ModuleUtils::IsBigEndian(this->raw_module));
}
