#include <Expression.h>

#include <Assembler.h>
#include "AssemblerTypes.h"
#include "AssemblyState.h"
#include "ExpressionResolver.h"
#include "StringUtil.h"

#include <regex>
#include <unordered_map>

namespace assembler {

using expression::NumericConstantExpression;

namespace {

class Operand {
public:
    Operand(std::string op, std::size_t index, std::string operand)
        : op(std::move(op)), operand(std::move(operand)) {}

    std::string AsString() {
        return operand;
    }

    std::unique_ptr<expression::Expression> AsExpression() {
        try {
            return ParseExpression(operand);
        } catch (std::runtime_error& e) {
            throw OperandException(op, index, e);
        }
    }

    void Fail(std::string requirement) {
        throw new OperandException(op, index, requirement);
    }

private:
    std::string op;
    std::size_t index;
    const std::string operand;
};

class OperandList {
public:
    OperandList(std::string op, std::vector<std::string> operand_strings) : op(std::move(op)) {
        operands.reserve(operand_strings.size());

        for (std::size_t i = 0; i < operand_strings.size(); i++) {
            operands.emplace_back(Operand(op, i, operand_strings.at(i)));
        }
    }

    Operand Get(std::size_t index) const {
        if (index >= operands.size()) {
            throw new OperandException(op, index,
                "Operation " + op + " missing positional operand " + std::to_string(index) + ".");
        }

        return operands.at(index);
    }

    std::size_t Count() const {
        return operands.size();
    }

private:
    std::vector<Operand> operands {};
    std::string op;
};

class Operation {
public:
    explicit Operation(const Entry& entry, const AssemblyState& state)
        : entry(entry), state(state) {}

    void RequireLabel() const {
        if (!entry.label)
            throw OperationException(entry.operation.value(), OperationException::Code::MissingLabel,
                BuildMessage("must have a label"));
    }

    void RequireNoOperands() const {
        if (entry.operands)
            throw OperationException(entry.operation.value(), OperationException::Code::UnexpectedOperands,
                BuildMessage("cannot have operands"));
    }

    void RequireNoLabel() const {
        if (entry.label)
            throw OperationException(entry.operation.value(), OperationException::Code::UnexpectedLabel,
                BuildMessage("cannot have a label"));
    }

    void RequireNoComment() const {
        if (entry.comment)
            throw OperationException(entry.operation.value(), OperationException::Code::UnexpectedComment,
                BuildMessage("cannot have a comment."));
    }

    void Fail(OperationException::Code code, std::string requirement) const {
        throw OperationException(entry.operation.value(), code, BuildMessage(requirement));
    }

    OperandList ParseOperands(std::regex delimiter = std::regex(",")) const {
        // TODO: catch split issues and return proper error
        auto parsed = Split(entry.operands.value_or(""), std::move(delimiter));
        return OperandList(entry.operation.value(), parsed);
    }

    const Entry& GetEntry() const {
        return entry;
    }

private:
    std::string BuildMessage(const std::string& requirement) const {
        return "Operation " + entry.operation.value_or("[null]") + requirement + ".";
    }

    const Entry& entry;
    const AssemblyState& state;
};

}

/**
 * Notes on the behavior of EQU.
 *
 * Ultra C Usage Guide:
 * > equ assigns a value to a symbolic name (the label).
 * >
 * > <label> may be any legal assembler label name.
 * > <expression> is the value to assign to the label. It may be an expression,
 * > a name, or a constant.
 *
 * > You can use the equ directive in any program section. The equ statement label
 * > name cannot have been defined previously. The operand cannot include a name that
 * > has not yet been defined (as yet undefined names whose definitions also use
 * > undefined names).
 *
 * > equ is normally used to define program symbolic constants, especially those used
 * > with instructions. Although the set directive is similar to equ, there are differences:
 * >  - Symbols defined by equ can be defined only once in the program.
 *
 * Additional Notes:
 *
 * Expression trees can include EQU values, which are considered EQU reference types (they
 * are not simply numeric constants).
 *
 * The label CAN be global, which will result in an entry in the External Definition
 * section of the ROF.
 *
 * Rules for global EQUs:
 * - They can NOT reference external names.
 * - To that end, they must be constant expressions. This is because to be included in
 *   the External Definition section of the ROF, they must be a 32-bit constant value.
 *
 * Rules for local EQUs:
 * - They CAN reference external names.
 * - They can only reference other local names that do NOT reference external names.
 *     TODO: why?
 *
 * TODO:
 *   - Try using an EQU that references a non-equ external reference. What happens?
 *     What is the value stored in the expression tree?
 *   - Does order matter? Equs *can* have expressions which use names defined later,
 *     lexically. But can order change anything?
 *
 * @param entry
 * @param state
 */
void Op_Equ(const Operation& operation, AssemblyState& state) {
    operation.RequireLabel();

    auto operands = operation.ParseOperands();
    auto expression = operands.Get(0).AsExpression();

    auto& entry = operation.GetEntry();
    auto& name = entry.label->name;
    if (state.GetSymbol(name)) {
        // note that Set *does* allow redefinition if the existing symbol is also for a previous Set
        operation.Fail(OperationException::Code::DuplicateSymbol,
            "Equ name must not already be defined.");
    }

    // Create Equ definition.
    state.psect.equs[name].value = std::move(expression);

    // Create symbol entry.
    object::SymbolInfo symbol_info;
    symbol_info.is_global = entry.label->is_global;
    symbol_info.type = object::SymbolInfo::Type::Equ;

    state.UpdateSymbol(entry.label.value(), symbol_info);
}

void Op_Psect(const Operation& operation, AssemblyState& state) {
    if (state.in_psect)
        operation.Fail(OperationException::Code::UnexpectedPSect,
            "must not be nested in another psect");

    if (state.in_vsect)
        operation.Fail(OperationException::Code::UnexpectedPSect,
            "may not appear with vsect");

    if (state.psect.tylan)
        operation.Fail(OperationException::Code::UnexpectedPSect,
            "psect already initialized. only 1 psect allowed per file");

    state.in_psect = true;
    PSect& p_sect = state.psect;

    auto operands = operation.ParseOperands();
    if (operands.Count() > 7) {
        // TODO: should this be an operand exception instead?
        operation.Fail(OperationException::Code::TooManyOperands, "must have <=7 operands");
    }

    if (operands.Count() == 0) {
        // Default name to "program" and rest of args to 0.
        p_sect.name = "program";
        p_sect.tylan = std::make_unique<NumericConstantExpression>(0);
        p_sect.revision = std::make_unique<NumericConstantExpression>(0);
        p_sect.edition = std::make_unique<NumericConstantExpression>(0);
        p_sect.stack = std::make_unique<NumericConstantExpression>(0);
        p_sect.entry_offset = std::make_unique<NumericConstantExpression>(0);
        p_sect.trap_handler_offset = std::make_unique<NumericConstantExpression>(0);
    } else {
        // Params are fully specified.

        // TODO: validation
        // > Any printable character may be used except a space or comma.
        // > However, the name must begin with a non-numeric character.
        p_sect.name = operands.Get(0).AsString();

        p_sect.tylan = operands.Get(1).AsExpression();
        p_sect.revision = operands.Get(2).AsExpression();
        p_sect.edition = operands.Get(3).AsExpression();
        p_sect.stack = operands.Get(4).AsExpression();
        p_sect.entry_offset = operands.Get(5).AsExpression();

        if (operands.Count() == 7) {
            p_sect.trap_handler_offset = operands.Get(6).AsExpression();
        }
    }
}

void Op_Vsect(const Operation& operation, AssemblyState& state) {
    if (state.in_vsect)
        operation.Fail(OperationException::Code::UnexpectedVSect,
            "must not be nested in another vsect");

    state.in_vsect = true;

    if (operation.ParseOperands().Count() > 0) {
        auto operands = operation.ParseOperands();
        auto remote = operands.Get(0);
        if (remote.AsString() != "remote")
            remote.Fail("vsect operand if specified must be 'remote'");
        state.in_remote_vsect = true;
    }
}

void Op_Ends(const Operation& operation, AssemblyState& state) {
    if (!state.in_psect && !state.in_vsect) {
        operation.Fail(OperationException::Code::UnexpectedEnds,
            "must be in a section");
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

void Op_End(const Operation& operation, AssemblyState& state) {
    operation.RequireNoLabel();

    // End of program signaled. Stop processing input lines.
    state.found_program_end = true;
}

void Op_Unimplemented(const Operation& operation, AssemblyState& state) {
    throw std::runtime_error("Directive is unimplemented: " + operation.GetEntry().operation.value_or(""));
}

typedef void (*DirectiveHandler)(const Operation&, AssemblyState&);
std::unordered_map<std::string, DirectiveHandler> directives = {
    { "equ", Op_Equ },
    { "psect", Op_Psect },
    { "vsect", Op_Vsect },
    { "ends", Op_Vsect },

    { "set", Op_Unimplemented },
    { "nam", Op_Unimplemented },
    { "ttl", Op_Unimplemented },
    { "opt", Op_Unimplemented },
    { "rept", Op_Unimplemented },
    { "endr", Op_Unimplemented },
    { "macro", Op_Unimplemented },
    { "endm", Op_Unimplemented },

    { "ifeq", Op_Unimplemented },
    { "ifne", Op_Unimplemented },
    { "iflt", Op_Unimplemented },
    { "ifle", Op_Unimplemented },
    { "ifgt", Op_Unimplemented },
    { "ifge", Op_Unimplemented },
    { "ifdef", Op_Unimplemented },
    { "ifndef", Op_Unimplemented },

    { "endc", Op_Unimplemented },
    { "use", Op_Unimplemented },
    { "spc", Op_Unimplemented },
};

bool HandleDirective(const Entry& entry, AssemblyState& state) {
    assert(entry.operation);

    Operation operation(entry, state);

    auto handler_kv = directives.find(entry.operation.value());
    if (handler_kv != directives.end()) {
        handler_kv->second(operation, state);
        return true;
    }

    return false;
}

}