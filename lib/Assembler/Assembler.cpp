#include "Assembler.h"
#include "AssemblerTypes.h"
#include "AssemblerTarget.h"
#include "Expression.h"
#include "ExpressionLexer.h"
#include "ExpressionParser.h"

#include "StringUtil.h"

#include <regex>
#include <functional>

namespace assembler {

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

    std::vector<VSect> vsects;
};

std::unique_ptr<Expression> ParseExpression(const std::string& expr_str) {
    auto lexer = ExpressionLexer(expr_str);
    auto parser = ExpressionParser(lexer);

    return parser.Parse();
}

Assembler::Assembler(std::unique_ptr<AssemblerTarget> target) : target(std::move(target)) {

}

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

void Assembler::Process(const std::vector<Entry> &listing) {
    size_t code_counter = 0;

    PSect p_sect {};
    bool in_psect = false;
    bool in_vsect = false;

    for (auto& entry : listing) {
        if (entry.operation) {
            auto require_no_label = [&entry]() {
                if (entry.label)
                    throw "Operation " + entry.operation.value_or("[null]") + " cannot have a label.";
            };

            auto require_no_comment = [&entry]() {
                if (entry.comment)
                    throw "Operation " + entry.operation.value_or("[null]") + "cannot have a comment.";
            };

            auto op = [&entry](const auto& str) {
                return entry.operation.value() == str;
            };

            if (op("psect"))
            {
                if (in_psect) throw "nested psects aren't allowed";
                if (in_vsect) throw "psect may not appear with vsect";
                if (p_sect.tylan) throw "psect already initialized. only 1 psect allowed per file";

                in_psect = true;
                ReadPSectParams(entry, p_sect);
            }
            else if (op("vsect"))
            {
                if (in_vsect) throw "nested vsects aren't allowed";
                in_vsect = true;
            }
            else if (op("ends"))
            {
                if (in_psect && in_vsect) {
                    // this is the only case for nesting sections.
                    in_vsect = false;
                } else {
                    in_psect = false;
                    in_vsect = false;
                }
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
            else if (op("end"))
            {
                require_no_label();
                // End of program signaled. Stop processing input lines.
                break;
            }
            else
            {
                // Pseudo or CPU instruction.
                auto instruction = target->EmitInstruction(entry);
            }
        }
    }
}

}