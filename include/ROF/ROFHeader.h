#pragma once

#include "SerializableStruct.h"

#include <array>
#include <string>
#include <vector>

namespace rof {

constexpr auto ROFSyncBytes = std::array { '\0', '\0', '\0', '\0'}; //TODO: find correct value

enum Type {
    Program = 1,
    Subroutine = 2,
    Multi = 3,
    Data = 4,
    ConfigStatusDescriptor = 5,

    TrapHandlerLibrary = 11,
    System = 12,
    FileManager = 13,
    DeviceDriver = 14,
    DeviceDescriptor = 15
};

enum Lang {
    ObjectCode = 1,
    ICode = 2,
    ICode_Pascal = 3,
    ICode_C = 4,
    ICode_Cobol = 5,
    ICode_Fortran = 6
};

struct VSect {
    bool isRemote;
};

struct PSect {
    std::string name;
    uint16_t
        tylan,
        revision,
        edition;
    uint32_t
        stack,
        entry_offset,
        trap_handler_offset;

    std::vector<VSect> vsects;

    Type GetType() {
        return static_cast<Type>(tylan >> 8U);
    }

    Lang GetLang() {
        return static_cast<Lang>(tylan & 0xFFU);
    }
};

using SerializableROFHeader = SerializableStruct<
    SerializableArray<char, 4>,
    SequenceOfType<uint16_t, 4>,
    SerializableArray<char, 6>,
    uint16_t,
    SequenceOfType<uint32_t, 11>,
    SequenceOfType<uint16_t, 3>
>;

struct ROFHeader : SerializableROFHeader {
    auto& SyncBytes() { return std::get<0>(*this); }
    auto& TypeLanguage() { return std::get<1>(*this); }
    auto& Revision() { return std::get<2>(*this); }
    auto& AsmValid() { return std::get<3>(*this); }
    auto& AsmVersion() { return std::get<4>(*this); }
    auto& AsmDate() { return std::get<5>(*this); }
    auto& Edition() { return std::get<6>(*this); }
    auto& StaticDataSize() { return std::get<7>(*this); }
    auto& InitializedDataSize() { return std::get<8>(*this); }

    __deprecated_msg("Reserved.")
    auto& ConstantDataSize() { return std::get<9>(*this); }

    auto& CodeSize() { return std::get<10>(*this); }
    auto& RequiredStackSize() { return std::get<11>(*this); }
    auto& OffsetToEntry() { return std::get<12>(*this); }
    auto& OffsetToUninitializedTrapHandler() { return std::get<13>(*this); }
    auto& RemoteStaticDataSizeRequired() { return std::get<14>(*this); }
    auto& RemoteInitializedDataSizeRequired() { return std::get<15>(*this); }

    __deprecated_msg("Reserved.")
    auto& RemoteConstantDataSizeRequired() { return std::get<16>(*this); }

    auto& DebugInfoSize() { return std::get<17>(*this); }
    auto& TargetCPU() { return std::get<18>(*this); }
    auto& CodeInfo() { return std::get<19>(*this); }

    __deprecated_msg("Reserved.")
    auto& HeaderExpansion() { return std::get<20>(*this); }

    // Note: null-terminated variable length module name immediately follows header.
};

}