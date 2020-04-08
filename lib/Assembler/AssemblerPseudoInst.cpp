#include <AssemblerTypes.h>
#include <Bitwise.h>
#include <Expression.h>

#include "AssemblyState.h"
#include "ExpressionResolver.h"

#include <iterator>
#include <unordered_map>
#include <ObjectFile.h>

namespace assembler {

namespace {

void CreateSymbolsHere(object::SymbolInfo::Type symbol_type, bool is_signed, AssemblyState& state) {
    auto counter = [symbol_type, &state]() {
        auto& counter = state.result.counter;
        switch (symbol_type) {
            case object::SymbolInfo::Type::UninitData:
                return counter.uninitialized_data;
            case object::SymbolInfo::Type::RemoteUninitData:
                return counter.remote_uninitialized_data;
            case object::SymbolInfo::Type::InitData:
                return counter.initialized_data;
            case object::SymbolInfo::Type::RemoteInitData:
                return counter.remote_initialized_data;
            case object::SymbolInfo::Type::Code:
                return counter.code;
        }
    };

    for (auto& label : state.pending_labels) {
        if (state.symbol_name_to_label.count(label.name) > 0) {
            throw "symbol is already defined!";
        }

        // Map symbol name to original label for assembly state.
        state.symbol_name_to_label[label.name] = label;

        // Add symbol to result object file.
        auto& symbols = label.is_global ? state.result.global_symbols : state.result.local_symbols;
        symbols[label.name] = object::SymbolInfo { symbol_type, is_signed, counter() };
    }
}

template<size_t Size, bool IsSigned>
void Op_DS(const Entry& entry, AssemblyState& state) {
    if (!state.in_psect || !state.in_vsect) {
        throw "must be in vsect within psect.";
    }

    if (!entry.operands) {
        throw "missing size operand";
    }

    // We need to resolve the size (count) expression *now*, since we must know by how much to increment data counter.
    auto size_expr = ParseExpression(entry.operands.value());
    auto count = ResolveExpression(*size_expr, state);
    auto increment = count * Size;

    auto& counter = state.in_remote_vsect
        ? state.result.counter.remote_uninitialized_data
        : state.result.counter.uninitialized_data;

    // Automatically align to even boundary for word and long (according to documentation).
    if constexpr (Size > 1) {
        counter = support::RoundToNextPow2Multiple(counter, 2);
    }

    // Assign label(s) to aligned relative address.
    if (entry.label) {
        state.pending_labels.emplace_back(entry.label.value());
    }

    CreateSymbolsHere(
        state.in_remote_vsect ? object::SymbolInfo::Type::RemoteUninitData : object::SymbolInfo::Type::UninitData,
        IsSigned,
        state);

    // Advance counter now that labels are mapped.
    counter += increment;
}

void Op_Align(const Entry& entry, AssemblyState& state) {
    if (!state.in_psect) {
        throw "align cannot exist outside of a psect.";
    }

    uint32_t alignment = 2; // default
    if (entry.operands) {
        auto expr = ParseExpression(entry.operands.value());

        auto as_numeric = dynamic_cast<expression::NumericConstantExpression *>(expr.get());
        if (as_numeric == nullptr) {
            throw "alignment must be numeric constant expression";
        }

        alignment = as_numeric->Value();

        if (!support::IsPow2(alignment)) {
            throw "alignment value must be a power of 2";
        }
    }

    auto align = [&alignment](auto &counter) {
        counter = support::RoundToNextPow2Multiple(counter, alignment);
    };

    auto& counter = state.result.counter;
    if (state.in_vsect) {
        // Align data counters
        align(state.in_remote_vsect ? counter.remote_initialized_data : counter.initialized_data);
        align(state.in_remote_vsect ? counter.remote_uninitialized_data : counter.uninitialized_data);
    } else {
        // Align code section.
        align(counter.code);
    }
}

void Op_Unimplemented(const Entry& entry, AssemblyState& state) {
    throw "pseudo instruction is unimplemented: " + entry.operation.value_or("");
}

typedef void (*PseudoInstFunc)(const Entry&, AssemblyState&);
std::unordered_map<std::string, PseudoInstFunc> pseudo_instructions = {
    { "align", Op_Align },
    { "com",   Op_Unimplemented },

    { "dc.b",  Op_Unimplemented },
    { "dc.sb", Op_Unimplemented },
    { "dc.w",  Op_Unimplemented },
    { "dc.sw", Op_Unimplemented },
    { "dc.l",  Op_Unimplemented },
    { "dc.sl", Op_Unimplemented },

    { "do.b",  Op_Unimplemented },
    { "do.sb", Op_Unimplemented },
    { "do.w",  Op_Unimplemented },
    { "do.sw", Op_Unimplemented },
    { "do.l",  Op_Unimplemented },
    { "do.sl", Op_Unimplemented },

    { "ds.b",  Op_DS<1, false> },
    { "ds.sb", Op_DS<1, true> },
    { "ds.w",  Op_DS<2, false> },
    { "ds.sw", Op_DS<2, true> },
    { "ds.l",  Op_DS<4, false> },
    { "ds.sl", Op_DS<4, true> },

    { "dz.b",  Op_Unimplemented },
    { "dz.sb", Op_Unimplemented },
    { "dz.w",  Op_Unimplemented },
    { "dz.sw", Op_Unimplemented },
    { "dz.l",  Op_Unimplemented },
    { "dz.sl", Op_Unimplemented },

    { "lo.b",  Op_Unimplemented },
    { "lo.sb", Op_Unimplemented },
    { "lo.w",  Op_Unimplemented },
    { "lo.sw", Op_Unimplemented },
    { "lo.l",  Op_Unimplemented },
    { "lo.sl", Op_Unimplemented },

    { "org",   Op_Unimplemented },
    { "tcall", Op_Unimplemented }
};
}

bool HandlePseudoInstruction(const Entry& entry, AssemblyState& state) {
    auto handler_kv = pseudo_instructions.find(entry.operation.value());
    if (handler_kv != pseudo_instructions.end()) {
        handler_kv->second(entry, state);
        return true;
    }

    return false;
}
}