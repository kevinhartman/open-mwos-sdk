#include "Assembler.h"
#include "AssemblerTypes.h"
#include "AssemblerTarget.h"
#include "Expression.h"
#include "ExpressionLexer.h"
#include "ExpressionParser.h"
#include "ExpressionResolver.h"

#include "StringUtil.h"
#include "Bitwise.h"

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>
#include <regex>
#include <unordered_map>
#include <variant>

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
};

std::unique_ptr<Expression> ParseExpression(const std::string& expr_str) {
    auto lexer = ExpressionLexer(expr_str);
    auto parser = ExpressionParser(lexer);

    return parser.Parse();
}

uint32_t ResolveExpression(const Expression& expression, const AssemblyState& state) {
    ReferenceResolver label_resolver = [&state](const Expression& e) {
        throw "unimplemented: labels";
        return 0;
    };

    ExpressionResolver resolver(label_resolver);
    expression.Accept(resolver);

    return resolver.result;
}

Assembler::Assembler(std::unique_ptr<AssemblerTarget> target) : target(std::move(target)) { }

Assembler::~Assembler() = default;

void ReadPSectParams(const Entry& entry, PSect& p_sect) {
    auto params = Split(entry.operands.value_or(""), std::regex(","));

    if (params.size() > 7) {
        throw "too many parameters to psect directive";
    }

    if (params.size() == 0) {
        // Default name to "program" and rest of args to 0.
        p_sect.name = "program";
        p_sect.tylan = std::make_unique<NumericConstantExpression>(0);
        p_sect.revision = std::make_unique<NumericConstantExpression>(0);
        p_sect.edition = std::make_unique<NumericConstantExpression>(0);
        p_sect.stack = std::make_unique<NumericConstantExpression>(0);
        p_sect.entry_offset = std::make_unique<NumericConstantExpression>(0);
        p_sect.trap_handler_offset = std::make_unique<NumericConstantExpression>(0);
    } else if (params.size() >= 6) {
        // Params are fully specified.

        // TODO: validation
        p_sect.name = params[0];

        p_sect.tylan = ParseExpression(params[1]);
        p_sect.revision = ParseExpression(params[2]);
        p_sect.edition = ParseExpression(params[3]);
        p_sect.stack = ParseExpression(params[4]);
        p_sect.entry_offset = ParseExpression(params[5]);

        if (params.size() == 7) {
            p_sect.trap_handler_offset = ParseExpression(params[6]);
        }
    } else {
        throw "not enough parameters to psect directive. Specify all (with optional trap), or none.";
    }
}


bool Assembler::HandleDirective(const Entry& entry, AssemblyState& state) {
    auto require_no_label = [&entry]() {
        if (entry.label)
            throw "Operation " + entry.operation.value_or("[null]") + " cannot have a label.";
    };

    auto require_no_comment = [&entry]() {
        if (entry.comment)
            throw "Operation " + entry.operation.value_or("[null]") + "cannot have a comment.";
    };

    bool handled = false;
    auto op = [&entry, &handled](const auto& str) {
        if (entry.operation.value() == str) {
            return (handled = true);
        }
        return false;
    };

    if (op("psect"))
    {
        if (state.in_psect) throw "nested psects aren't allowed";
        if (state.in_vsect) throw "psect may not appear with vsect";
        if (state.psect.tylan) throw "psect already initialized. only 1 psect allowed per file";

        state.in_psect = true;
        ReadPSectParams(entry, state.psect);
    }
    else if (op("vsect"))
    {
        if (state.in_vsect) throw "nested vsects aren't allowed";
        state.in_vsect = true;

        if (entry.operands) {
            if (entry.operands.value() != "remote") throw "invalid operand to vsect directive";
            state.in_remote_vsect = true;
        }
    }
    else if (op("ends"))
    {
        if (!state.in_psect && !state.in_vsect) {
            throw "not in section";
        }

        if (state.in_psect && state.in_vsect) {
            // If we were in both, we must be exiting the vsect.
            state.in_vsect = false;
        } else {
            // We're exiting either a vsect or psect. In either case, we're no longer in any section.
            state.in_psect = false;
            state.in_vsect = false;
        }
    }
    else if (op("equ")) {
        throw "unimplemented";
    }
    else if (op("set")) {
        throw "unimplemented";
    }
    else if (op("nam")) {
        throw "unimplemented";
    }
    else if (op("ttl")) {
        throw "unimplemented";
    }
    else if (op("opt")) {
        throw "unimplemented";
    }
    else if (op("rept")) {
        throw "unimplemented.";
    }
    else if (op("endr")) {
        throw "unimplemented.";
    }
    else if (op("macro"))
    {
        throw "unimplemented.";
    }
    else if (op("endm"))
    {
        throw "unimplemented.";
    }
    else if (op("ifeq") || op("ifne") || op("iflt") || op("ifle") || op("ifgt") || op("ifge") || op("ifdef") || op("ifndef"))
    {
        throw "unimplemented.";
    }
    else if (op("endc"))
    {
        throw "unimplemented.";
    }
    else if (op("use"))
    {
        throw "unimplemented.";
    }
    else if (op("spc")) {
        throw "unimplemented.";
    }
    else if (op("end"))
    {
        require_no_label();
        // End of program signaled. Stop processing input lines.
        state.found_program_end = true;
    }

    return handled;
}

void CreateSymbolsHere(SymbolInfo::Type symbol_type, bool is_signed, AssemblyState& state) {
    auto counter = [symbol_type, &state]() {
        switch (symbol_type) {
            case SymbolInfo::Type::UninitData:
                return state.counter.uninitialized_data;
            case SymbolInfo::Type::RemoteUninitData:
                return state.counter.remote_uninitialized_data;
            case SymbolInfo::Type::InitData:
                return state.counter.initialized_data;
            case SymbolInfo::Type::RemoteInitData:
                return state.counter.remote_initialized_data;
            case SymbolInfo::Type::Code:
                return state.counter.code;
        }
    };

    for (auto& label : state.pending_labels) {
        if (state.symbols.count(label.name) > 0) {
            throw "symbol is already defined!";
        }

        state.symbols[label.name] = { label, SymbolInfo { symbol_type, is_signed, counter() } };
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
        ? state.counter.remote_uninitialized_data
        : state.counter.uninitialized_data;

    // Automatically align to even boundary for word and long (according to documentation).
    if constexpr (Size > 1) {
        counter = support::RoundToNextPow2Multiple(counter, 2);
    }

    // Assign label(s) to aligned relative address.
    if (entry.label) {
        state.pending_labels.emplace_back(entry.label.value());
    }

    CreateSymbolsHere(
        state.in_remote_vsect ? SymbolInfo::Type::RemoteUninitData : SymbolInfo::Type::UninitData,
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

        auto as_numeric = dynamic_cast<NumericConstantExpression *>(expr.get());
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

    if (state.in_vsect) {
        // Align data counters
        align(state.in_remote_vsect ? state.counter.remote_initialized_data : state.counter.initialized_data);
        align(state.in_remote_vsect ? state.counter.remote_uninitialized_data : state.counter.uninitialized_data);
    } else {
        // Align code section.
        align(state.counter.code);
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

bool Assembler::HandlePseudoInstruction(const Entry& entry, AssemblyState& state) {
    auto handler_kv = pseudo_instructions.find(entry.operation.value());
    if (handler_kv != pseudo_instructions.end()) {
        handler_kv->second(entry, state);
        return true;
    }

    return false;
}

void Assembler::Process(const std::vector<Entry> &listing) {
    AssemblyState state {};

    for (auto& entry : listing) {
        if (state.found_program_end) {
            return;
        }

        if (entry.operation) {
            if (HandleDirective(entry, state)) continue;
            if (HandlePseudoInstruction(entry, state)) continue;

            // This must be a CPU instruction.
            if (!(state.in_psect && !state.in_vsect)) {
                throw "target instruction must be inside the psect";
            }

            auto instruction = target->EmitInstruction(entry);

            // Add instruction to code section.
            state.psect.code[state.counter.code] = std::move(instruction);
            state.counter.code += instruction.size;
        } else {
            if (entry.label) {
                // Remember the label so it can be mapped to the next appropriate counter value.
                state.pending_labels.emplace_back(entry.label.value());
            }
        }
    }
}

}