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

#include <iomanip>

#include "ModuleInfoPrinter.hpp"

namespace module::util {

template<class T>
void PrintHex(T printee, std::ostream& output_stream) {
    std::ios_base::fmtflags backup_flags = output_stream.flags();
    char backup_fill = output_stream.fill();

    output_stream << "0x";
    output_stream << std::hex << std::uppercase << std::internal << std::setfill('0');;
    output_stream << std::setw(sizeof(T) * 2);
    output_stream << printee;

    output_stream.flags(backup_flags);
    output_stream.fill(backup_fill);
}

void PrintHex(uint8_t printee, std::ostream& output_stream) {
    std::ios_base::fmtflags backup_flags = output_stream.flags();
    char backup_fill = output_stream.fill();

    output_stream << "0x";
    output_stream << std::hex << std::uppercase;
    output_stream << (int)printee;

    output_stream.flags(backup_flags);
    output_stream.fill(backup_fill);
}

void PrintModuleInfo(Module& module, std::ostream& output_stream) {
    auto header = module.GetHeader();

    output_stream << "Sync Bytes: ";
    PrintHex(header->SyncBytes(), output_stream);
    output_stream << " (" << (module.IsBigEndian() ? "big" : "little") << ") " << std::endl;

    output_stream << "System Revision: ";
    output_stream << header->SystemRevision() << std::endl;

    output_stream << "Size: ";
    output_stream << header->Size() << std::endl;

    output_stream << "Owner ID: ";
    output_stream << header->Owner() << std::endl;

    output_stream << "Name: ";
    output_stream << module.GetName() << std::endl;

    output_stream << "Name Offset: ";
    PrintHex(header->OffsetToName(), output_stream);
    output_stream << std::endl;

    output_stream << "Permissions: ";
    PrintHex(header->Access(), output_stream);
    output_stream << std::endl;

    output_stream << "Type: ";
    output_stream << (uint16_t)(header->TypeLanguage() >> 8U) << std::endl;

    output_stream << "Language: ";
    output_stream << (uint16_t)(header->TypeLanguage() & 0x00FFU) << std::endl;

    output_stream << "Attributes: ";
    output_stream << (uint16_t)(header->AttRev() >> 8U) << std::endl;

    output_stream << "Attributes Revision: ";
    output_stream << (uint16_t)(header->AttRev() & 0x00FFU) << std::endl;

    output_stream << "Vendor Edition: ";
    output_stream << header->Edition() << std::endl;

    output_stream << "Hardware Needs: ";
    PrintHex(header->HardwareNeeds(), output_stream);
    output_stream << std::endl;

    output_stream << "Shared Data Offset: ";
    PrintHex(header->OffsetToShared(), output_stream);
    output_stream << std::endl;

    output_stream << "Symbol Table Offset: ";
    PrintHex(header->OffsetToSymbol(), output_stream);
    output_stream << std::endl;

    output_stream << "Exec Entry Point: ";
    PrintHex(header->OffsetToExec(), output_stream);
    output_stream << std::endl;

    output_stream << "Exception Entry Point: ";
    PrintHex(header->OffsetToExcept(), output_stream);
    output_stream << std::endl;

    output_stream << "Program Data Size: ";
    output_stream << header->SizeOfData() << std::endl;

    output_stream << "Min Stack Size: ";
    output_stream << header->MinStackSize() << std::endl;

    output_stream << "Initialization Data Header Offset: ";
    PrintHex(header->InitializedDataOffset(), output_stream);
    output_stream << std::endl;

    InitDataHeader initDataHeader = module.GetInitializationDataHeader();

    output_stream << "Initialization Data Offset: ";
    PrintHex(initDataHeader.GetOffset(), output_stream);
    output_stream << std::endl;

    output_stream << "Initialization Data Byte Count: ";
    output_stream << initDataHeader.GetByteCount() << std::endl;

//    output_stream << header.offset_idata << std::endl;
//    output_stream << header.offset_idref << std::endl;
//    output_stream << header.offset_init << std::endl;
//    output_stream << header.offset_term << std::endl;
    output_stream << header->DataBias() << std::endl;
    output_stream << header->CodeBias() << std::endl;
//    output_stream << header.link_ident << std::endl;
//    output_stream << header.parity << std::endl;
    output_stream << std::endl;
    std::flush(output_stream);
}

}
