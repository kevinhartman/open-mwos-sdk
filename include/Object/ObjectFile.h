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

struct SymbolInfo {
    enum Type {
        Code,
        InitData,
        UninitData,
        RemoteInitData,
        RemoteUninitData
    } type;

    bool is_signed; // TODO: symbols don't have sign in ROF. Why did I put this here?
    std::optional<uint32_t> value;
};

struct ObjectFile {
    uint16_t assembler_version;
    uint64_t assembly_time_epoch;

    std::string name;
    uint16_t tylan;
    uint16_t revision;
    uint16_t edition;

    u_int32_t stack_size;
    u_int32_t entry_offset;
    u_int32_t trap_handler_offset;

    struct {
        size_t code;
        size_t initialized_data;
        size_t uninitialized_data;
        size_t remote_initialized_data;
        size_t remote_uninitialized_data;
    } counter;

    std::map<std::string, SymbolInfo> global_symbols {};
    std::map<std::string, SymbolInfo> local_symbols {};
};

}