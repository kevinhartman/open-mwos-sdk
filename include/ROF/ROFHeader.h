#pragma once

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

struct ROFHeader {
    std::array<char, 4> sync_bytes;
    uint16_t
        tylan,
        revision,
        asm_valid,
        asm_version;
    std::array<char, 6> asm_date;
    uint16_t edition;
    uint32_t
        static_data_size,
        initialized_data_size,
        const_data_size, // unused
        code_size,
        stack_size_required,
        entry_offset,
        uninitialized_trap_offset,
        remote_static_data_storage_size,
        remote_initialized_data_storage_size,
        remote_const_data_storage_size, // unused
        debug_info_size;
    uint16_t target_cpu; // upper: family, lower: cpu code
    uint16_t code_info; // probably a bitfield describing code. TODO: find compiled ROF with "threaded == 1"
    uint16_t header_expansion; // unused
    // Note: null-terminated variable length module name immediately follows header.
} __attribute__((packed));

}