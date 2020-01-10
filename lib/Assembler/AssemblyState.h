#pragma once

#include "AssemblerTarget.h"
#include "AssemblerTypes.h"
#include "Expression.h"
#include "ROFObjectFile.h"

#include <map>
#include <variant>
#include <vector>

namespace assembler {
typedef size_t local_offset;

// TODO: delete. This has been replaced by use of std::variant.
//struct DataValue {
//    bool is_str;
//    union {
//        std::string str;
//        std::unique_ptr<Expression> expr;
//    } value;
//};

struct Data {
    size_t size;
    bool is_signed = false;
    std::variant<std::string, std::unique_ptr<Expression>> value;
};

struct VSect {
    bool isRemote;
};

struct PSect {
    std::string name;
    std::unique_ptr<Expression>
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

struct AssemblyState {
    bool found_program_end = false;
    struct {
        size_t code;
        size_t initialized_data;
        size_t uninitialized_data;
        size_t remote_initialized_data;
        size_t remote_uninitialized_data;
    } counter;

    bool in_psect = false;
    bool in_vsect = false;
    bool in_remote_vsect = false;
    std::vector<Label> pending_labels {};

    PSect psect {};
    std::vector<VSect> root_vsects {};
    std::map<std::string, std::tuple<Label, SymbolInfo>> symbols {};

    rof::ROFObjectFile result {};
};
}