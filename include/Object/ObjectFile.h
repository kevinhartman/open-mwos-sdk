#pragma once

#include <Endian.h>
#include <Expression.h>

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>
#include <ctime>

namespace object {

/**
 * These may seem like ROF-specific values, but they are here because they're part of the *public* interface
 * that assembly programmers are aware of (they come directly from user-supplied strings in all assemblers).
 */
enum class Type {
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

enum class Lang {
    ObjectCode = 1,
    ICode = 2,
    ICode_Pascal = 3,
    ICode_C = 4,
    ICode_Cobol = 5,
    ICode_Fortran = 6
};

enum class CpuTarget {
    os9_68k,
    os9k_386,
    os9k_68k,
    os9k_ppc,
    os9k_sh5m,
    os9k_mips,
    os9k_sh,
    os9k_arm,
    os9k_sparc,
    os9k_rce,
    os9k_sh4,
    os9k_armbe,
    os9k_armv5,
    os9k_sh4a
};

struct VSect {
    bool isRemote;
};

struct SymbolInfo {
    enum Type {
        Code,
        Equ,
        Set,
        InitData,
        UninitData,
        RemoteInitData,
        RemoteUninitData
    } type;

    bool is_global;

    // Will be nullopt for equ and set
    std::optional<uint32_t> value;
};

struct ExpressionMapping {
    size_t offset;
    size_t bit_count;
    std::shared_ptr<expression::Expression> expression;
};

struct MemoryValue {
    union {
        std::array<uint8_t, sizeof(uint64_t)> raw;
        uint64_t u64;
        uint32_t u32;
        uint16_t u16;
        uint8_t u8;
    } data {};
    size_t size {};
    bool is_signed {};
    std::vector<ExpressionMapping> expr_mappings {};
};

//struct EquDefinition {
//    std::unique_ptr<expression::Expression> value;
//};

struct SetDefinition {
    std::unique_ptr<expression::Expression> value;
};

typedef size_t local_offset;
struct PSect {
    std::vector<VSect> vsects {};

    std::map<local_offset, MemoryValue> initialized_data {};
    std::map<local_offset, MemoryValue> remote_initialized_data {};
    std::map<local_offset, MemoryValue> code_data {};

    std::map<std::string, object::SymbolInfo> symbols {};
};

struct ObjectFile {
    uint16_t assembler_version;
    std::time_t assembly_time;

    std::string name;
    uint16_t tylan;
    uint16_t revision;
    uint16_t edition;

    u_int32_t stack_size;
    u_int32_t entry_offset;
    u_int32_t trap_handler_offset;

    CpuTarget cpu_target;
    support::Endian endian;

    struct {
        size_t code;
        size_t initialized_data;
        size_t uninitialized_data;
        size_t remote_initialized_data;
        size_t remote_uninitialized_data;
    } counter;

    PSect psect {};
    std::vector<VSect> root_vsects {};

    std::map<std::string, SymbolInfo> global_symbols {};
    std::map<std::string, SymbolInfo> local_symbols {};
};

}