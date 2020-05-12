#pragma once

#include "AssemblerTarget.h"
#include "AssemblerTypes.h"
#include <Expression.h>
#include <ObjectFile.h>

#include <map>
#include <optional>
#include <vector>

namespace assembler {

/**
 * Represents a constant data declaration (i.e. dc.*).
 *
 * The value can be either an expression, or a character string constant.
 *
 * > Character string constants can be any sequence of printable ASCII characters
 * > enclosed in double quotes. For dc.w and dc.l, a string constant is padded
 * > with zeroes on the right end if it does not fill the final word or long word.
 * > Therefore, dc.b is the most natural format for strings.
 *
 * Note to future me: local usage references need to copy the properties such
 * as sign and size to themselves, since actual symbols in the ROF only know
 */
struct DataDefinition {
    size_t size;
    bool is_signed = false;
    std::variant<std::string, std::unique_ptr<expression::Expression>> value;
};

struct VSect {
    bool isRemote;
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

    std::map<local_offset, DataDefinition> initialized_data {};
    std::map<local_offset, DataDefinition> uninitialized_data {};

    std::map<local_offset, DataDefinition> remote_initialized_data {};
    std::map<local_offset, DataDefinition> remote_uninitialized_data {};

    std::map<local_offset, Instruction> code {};

    std::map<std::string, object::SymbolInfo> symbols {};
};

struct AssemblyState {
    inline std::optional<object::SymbolInfo> GetSymbol(std::string name) {
        auto symbol_itr = result.local_symbols.find(name);
        if (symbol_itr != result.local_symbols.end()) {
            return symbol_itr->second;
        }

        symbol_itr = result.global_symbols.find(name);
        if (symbol_itr != result.global_symbols.end()) {
            return symbol_itr->second;
        }

        return std::nullopt;
    }

    inline void UpdateSymbol(const Label& label, const object::SymbolInfo& symbol_info) {
        symbol_name_to_label[label.name] = label;

        auto& symbols = label.is_global ? result.global_symbols : result.local_symbols;
        symbols[label.name] = symbol_info;
    }

    bool found_program_end = false;

    bool in_psect = false;
    bool in_vsect = false;
    bool in_remote_vsect = false;
    std::vector<Label> pending_labels {};
    std::map<std::string, Label> symbol_name_to_label;

    PSect psect {};
    std::vector<VSect> root_vsects {};
    object::ObjectFile result {};
};
}