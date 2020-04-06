#include "Assembler.h"
#include "AssemblerTypes.h"
#include "AssemblerTarget.h"
#include "AssemblyState.h"
#include "ExpressionResolver.h"
#include "Numeric.h"
#include "ObjectFile.h"

#include <limits>
#include <vector>

namespace assembler {

extern bool HandlePseudoInstruction(const Entry &entry, AssemblyState &state);
extern bool HandleDirective(const Entry &entry, AssemblyState &state);

namespace {

    // TODO: currently, PSect will contain straight up unresolved expressions.
    //   That's why we need to resolve these values after processing the TU.
    //   Without thinking, I moved PSect to object::ObjectFile, but that might've
    //   been a mistake, since perhaps unresolved expressions should not leave
    //   the assembler except for serialization. I need to think through that
    //   abstraction.
    //
    //   Update: for now, the approach will be to keep those unresolved expressions
    //   inside object::ObjectFile, but we will resolve them here too.
void SetObjectInfo(const AssemblyState& state, object::ObjectFile& object) {
    const auto& psect = state.result.psect;

    const auto expr_16bit = [](std::string name, auto value) {
        if (value > std::numeric_limits<uint16_t>::max()) {
            throw name + " must be 16-bit expression!";
        }

        return value;
    };

    const auto expr_32bit = [](std::string name, auto value) {
        if (value > std::numeric_limits<uint32_t>::max()) {
            throw name + " must be 32-bit expression!";
        }

        return value;
    };

    object.name = psect.name;

    object.tylan = expr_16bit("tylan", ResolveExpression(*psect.tylan, state));
    object.revision = expr_16bit("attrev", ResolveExpression(*psect.revision, state));
    object.edition = expr_16bit("edition", ResolveExpression(*psect.edition, state));

    object.stack_size = expr_32bit("stack", ResolveExpression(*psect.stack, state));
    object.entry_offset = expr_32bit("entrypt", ResolveExpression(*psect.entry_offset, state));
    object.trap_handler_offset = expr_32bit("trapent", ResolveExpression(*psect.trap_handler_offset, state));
}

}

std::unique_ptr<object::ObjectFile> Assembler::CreateResult(AssemblyState& state) {
    auto object_file = std::make_unique<object::ObjectFile>();

    // Set assembler info
    object_file->assembler_version = assembler_version;
    object_file->assembly_time_epoch = 0; // TODO: get current time as epoch.

    SetObjectInfo(state, *object_file);

    return object_file;
}

Assembler::Assembler(uint16_t assembler_version, std::unique_ptr<AssemblerTarget> target)
    : assembler_version(assembler_version), target(std::move(target)) { }
Assembler::~Assembler() = default;

bool Assembler::HandlePseudoInstruction(const Entry& entry, AssemblyState& state) {
    return assembler::HandlePseudoInstruction(entry, state);
}

bool Assembler::HandleDirective(const Entry& entry, AssemblyState& state) {
    return assembler::HandleDirective(entry, state);
}

std::unique_ptr<object::ObjectFile> Assembler::Process(const std::vector<Entry> &listing) {
    AssemblyState state {};

    for (auto& entry : listing) {
        if (state.found_program_end) {
            return CreateResult(state);
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
            state.result.psect.code[state.result.counter.code] = std::move(instruction);
            state.result.counter.code += instruction.size;
        } else {
            if (entry.label) {
                // Remember the label so it can be mapped to the next appropriate counter value.
                state.pending_labels.emplace_back(entry.label.value());
            }
        }
    }

    return CreateResult(state);
}

}