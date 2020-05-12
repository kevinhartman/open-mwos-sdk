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

class Operands {
public:
    Operands(OperationId op_id, std::vector<std::string> operands) : op_id(op_id), operands(std::move(operands)) {}

    std::size_t Count() const {
        return operands.size();
    }

    std::string GetString(std::size_t index) {
        RequireOperand(index);
        return operands.at(index);
    }

    std::unique_ptr<expression::Expression> GetExpression(std::size_t index) {
        RequireOperand(index);

        try {
            return ParseExpression(operands.at(index));
        } catch (std::runtime_error& e) {
            throw OperandException(op_id, index, e);
        }
    }

private:
    void RequireOperand(std::size_t index) {
        if (index >= operands.size()) {
            throw OperandException(op_id, index, "Operation missing expected operand.");
        }
    }

    OperationId op_id;
    const std::vector<std::string> operands;
};

class Operation {
public:
    explicit Operation(const Entry& entry, const AssemblyState& state)
        : entry(entry), state(state) {}

    void RequireLabel() const {
        if (!entry.label)
            throw OperationException(OperationId::Equ, OperationException::Code::MissingLabel,
                BuildMessage("must have a label"));
    }

    void RequireNoOperands() const {
        if (entry.operands)
            throw OperationException(OperationId::Equ, OperationException::Code::UnexpectedOperands,
                BuildMessage("cannot have operands"));
    }

    void RequireNoLabel() const {
        if (entry.label)
            throw OperationException(OperationId::Equ, OperationException::Code::UnexpectedLabel,
                BuildMessage("cannot have a label"));
    }

    void RequireNoComment() const {
        if (entry.comment)
            throw OperationException(OperationId::Equ, OperationException::Code::UnexpectedComment,
                BuildMessage("cannot have a comment."));
    }

    void Fail(OperationException::Code code, std::string requirement) const {
        throw OperationException(OperationId::Equ, code, BuildMessage(requirement));
    }

    Operands ParseOperands(std::regex delimiter = std::regex(",")) const {
        auto parsed = Split(entry.operands.value(), delimiter);
        return Operands(OperationId::Equ, std::move(parsed));
    }

    const Entry& GetEntry() const {
        return entry;
    }

private:
    std::string BuildMessage(std::string requirement) const {
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

    Operands operands = operation.ParseOperands();
    auto expression = operands.GetExpression(0);

    auto& entry = operation.GetEntry();
    auto& name = entry.label->name;
    if (state.GetSymbol(name)) {
        // note that Set *does* allow redefinition if the existing symbol is also for a previous Set
        throw OperationException(OperationId::Equ, OperationException::Code::DuplicateSymbol,
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
        operation.Fail(OperationException::Code::UnexpectedPSect, "must not be nested in another psect");

    if (state.in_vsect)
        operation.Fail(OperationException::Code::UnexpectedPSect, "may not appear with vsect");

    if (state.psect.tylan)
        operation.Fail(OperationException::Code::UnexpectedPSect,
            "psect already initialized. only 1 psect allowed per file");

    state.in_psect = true;
    PSect& p_sect = state.psect;

    auto operands = operation.ParseOperands();
    if (operands.Count() > 7) {
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
        p_sect.name = operands.GetString(0);

        p_sect.tylan = operands.GetExpression(1);
        p_sect.revision = operands.GetExpression(2);
        p_sect.edition = operands.GetExpression(3);
        p_sect.stack = operands.GetExpression(4);
        p_sect.entry_offset = operands.GetExpression(5);

        if (operands.Count() == 7) {
            p_sect.trap_handler_offset = operands.GetExpression(6);
        }
    }
}

void Op_Vsect(const Operation& operation, AssemblyState& state) {
    if (state.in_vsect)
        operation.Fail(OperationException::Code::UnexpectedVSect,
            "must not be nested in another vsect");

    state.in_vsect = true;

    if (operation.GetEntry().operands) {
        auto operands = operation.ParseOperands();
        if (operands.GetString(0) != "remote") throw OperandException()"invalid operand to vsect directive";
        state.in_remote_vsect = true;
    }
}

typedef void (*DirectiveHandler)(const Operation&, AssemblyState&);
std::unordered_map<std::string, DirectiveHandler> directives = {
    { "equ", Op_Equ },
    { "psect", Op_Psect }
};

bool HandleDirective(const Entry& entry, AssemblyState& state) {
    assert(entry.operation);

    Operation operation(entry, state);

    auto handler_kv = directives.find(entry.operation.value());
    if (handler_kv != directives.end()) {
        handler_kv->second(operation, entry, state);
        return true;
    }

    return false;
}

bool HandleDirective1(const Entry& entry, AssemblyState& state) {
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
        ReadPSectParams(entry, state);
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
        /*
         * Equs and set declarations can actually be used externally (from other ROFs).
         */
        Op_Equ(entry, state);
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
}