#include <Expression.h>

#include "AssemblerTypes.h"
#include "AssemblyState.h"
#include "ExpressionResolver.h"
#include "StringUtil.h"

#include <regex>

namespace assembler {

using expression::NumericConstantExpression;

namespace {

void ReadPSectParams(const Entry& entry, AssemblyState& state) {
    auto params = Split(entry.operands.value_or(""), std::regex(","));
    object::PSect& p_sect = state.result.psect;
    
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
}

bool HandleDirective(const Entry& entry, AssemblyState& state) {
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
        if (state.result.psect.tylan) throw "psect already initialized. only 1 psect allowed per file";

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
}