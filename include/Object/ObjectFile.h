#pragma once

#include <Expression.h>

#include <cstdint>
#include <map>
#include <string>
#include <variant>
#include <vector>

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

struct Data {
    size_t size;
    bool is_signed = false;
    std::variant<std::string, std::unique_ptr<expression::Expression>> value;
};

struct VSect {
    bool isRemote;
};

struct ExpressionMapping {
    size_t offset;
    size_t bit_count;
    std::shared_ptr<expression::Expression> expression;
};

struct Instruction {
    union {
        uint8_t raw[sizeof(uint64_t)];
        uint64_t u64;
        uint32_t u32;
        uint16_t u16;
        uint8_t u8;
    } data;
    size_t size;
    std::vector<ExpressionMapping> expr_mappings;
};

typedef size_t local_offset;
struct PSect {
    std::string name;
    std::unique_ptr<expression::Expression>
        tylan,
        revision,
        edition,
        stack,
        entry_offset,
        trap_handler_offset;

    std::vector<VSect> vsects {};

    std::map<local_offset, Data> initialized_data {};
    std::map<local_offset, Data> uninitialized_data {};

    std::map<local_offset, Data> remote_initialized_data {};
    std::map<local_offset, Data> remote_uninitialized_data {};

    std::map<local_offset, Instruction> code {};
};

struct SymbolInfo {
    enum Type {
        Code,
        InitData,
        UninitData,
        RemoteInitData,
        RemoteUninitData
    } type;

    bool is_signed;
    std::optional<uint32_t> value;
};

struct ObjectFile {
    uint16_t assembler_version;
    uint64_t assembly_time_epoch;

    std::string name;
    uint16_t tylan;
    uint16_t revision;
    uint16_t edition;

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