#include "AssemblerDirectiveHandler.h"

#include <Expression.h>

#include <Assembler.h>
#include "AssemblerTypes.h"
#include "AssemblyState.h"
#include "Operation.h"

#include <unordered_map>

namespace {

using expression::NumericConstantExpression;

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
 * Expression trees can include local EQU values, which are not just numeric constants, but
 * are instead references of type EQU. The value of the reference must be a numeric constant.
 *
 * TODO:
 *  This means that if the referenced local EQU itself references an external name, the local
 *  EQU's expression probably needs to be expanded into the callsite reference, so that the
 *  EQU's external reference becomes the callsite's external reference.
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
 *     => the EQU's expression must get injected into the expressions that use it.
 * - They can only reference other local names that do NOT reference external names.
 *     TODO: why?
 *
 * TODO:
 *   - Try using an EQU that references a non-equ external reference. What happens?
 *     What is the value stored in the expression tree?
 *   - Try using an EQU that references an external name from a constexpr.
 *   - Should EQU support multiple labels?
 *
 * @param entry
 * @param state
 */
void Op_Equ(const Operation& operation, AssemblyState& state) {
    operation.RequireLabel();

    auto operands = operation.ParseOperands();
    auto expression = operands.Get(0, "expression").AsExpression();

    auto& entry = operation.GetEntry();
    auto& name = entry.label->name;
    if (state.GetSymbol(name)) {
        // note that Set *does* allow redefinition if the existing symbol is also for a previous Set
        operation.Fail(OperationException::Code::DuplicateSymbol,
            "Equ name must not already be defined.");
    }

    // Create Equ definition.
    // Note: these are for use by expression trees in other operations in this translation unit. We don't need to enqueue
    //       any sort of evaluation for these, since referencing ops will do that.
    state.psect.equs[name].value = std::move(expression);

    if (entry.label->is_global) {
        // For a global EQU, we must create an external definition. We do this for now by
        // creating a "global" symbol entry with Type::EQU.
        // TODO:
        //  - It might be better to instead have a separate list of external definitions in object::ObjectFile.
        //    This is because other ROF tools don't consider Equs and Set to be symbols (but assembler's -s option seems to),
        //    so perhaps we should treat them as just external defs.
        //  - We only add such symbols for global Equs because they cannot reference external names, and thus
        //    we can resolve them to constants in the second pass. Perhaps we should add symbols for local equs too,
        //    since the assembler's -s option shows local equs too. But, we would need to track them differently,
        //    since they can include external references, and thus cannot be encoded as a constant integer.
        state.second_pass_queue.emplace_back([&label = entry.label](AssemblyState &state) {
            object::SymbolInfo symbol_info;
            symbol_info.is_global = label->is_global;
            symbol_info.type = object::SymbolInfo::Type::Equ;

            state.UpdateSymbol(label.value(), symbol_info);
        });
    }
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
    auto& p_sect = state.psect;

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
        p_sect.name = operands.Get(0, "name").AsString();

        p_sect.tylan = operands.Get(1, "typelang").AsExpression();
        p_sect.revision = operands.Get(2, "attrev").AsExpression();
        p_sect.edition = operands.Get(3, "edition").AsExpression();
        p_sect.stack = operands.Get(4, "stacksize").AsExpression();
        p_sect.entry_offset = operands.Get(5, "entrypt").AsExpression();

        if (operands.Count() == 7) {
            p_sect.trap_handler_offset = operands.Get(6, "trapent").AsExpression();
        }
    }

    state.second_pass_queue.emplace_back([&p_sect = state.psect](AssemblyState& state) {
        auto& result = state.result;

        // TODO: remove state parameter. Note the RHS exprs are using p_sect from capture
        result.name = p_sect.name;
        result.tylan = ResolveExpression(*p_sect.tylan, state);
        result.revision = ResolveExpression(*p_sect.revision, state);
        result.edition = ResolveExpression(*p_sect.edition, state);
        result.stack_size = ResolveExpression(*p_sect.stack, state);
        result.entry_offset = ResolveExpression(*p_sect.entry_offset, state);
        result.trap_handler_offset = ResolveExpression(*p_sect.trap_handler_offset, state);
    });
}

void Op_Vsect(const Operation& operation, AssemblyState& state) {
    if (state.in_vsect)
        operation.Fail(OperationException::Code::UnexpectedVSect,
            "must not be nested in another vsect");

    state.in_vsect = true;

    if (operation.ParseOperands().Count() > 0) {
        auto operands = operation.ParseOperands();
        auto remote = operands.Get(0, "remote");
        if (remote.AsString() != "remote")
            remote.Fail("vsect operand if specified must be the string literal 'remote'");
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

    // TODO: should we enqueue a task to emit vsect or psect processing?
}

void Op_End(const Operation& operation, AssemblyState& state) {
    operation.RequireNoLabel();

    // End of program signaled. Stop processing input lines.
    state.found_program_end = true;
}

void Op_Unimplemented(const Operation& operation, AssemblyState& state) {
    throw std::runtime_error("Directive is unimplemented: " + operation.GetEntry().operation.value_or(""));
}

}

namespace assembler {

typedef void (*DirectiveHandler)(const Operation&, AssemblyState&);
std::unordered_map<std::string, DirectiveHandler> directives = {
    { "equ", Op_Equ },
    { "psect", Op_Psect },
    { "vsect", Op_Vsect },
    { "ends", Op_Ends },

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
    { "end", Op_End},
};

AssemblerDirectiveHandler::~AssemblerDirectiveHandler() = default;

bool AssemblerDirectiveHandler::Handle(const Entry& entry, AssemblyState& state) {
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