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
void SetObjectInfo(const AssemblyState& state, object::ObjectFile& object) {
    object.name = state.result.psect.name;

    auto tylan = ResolveExpression(*state.result.psect.tylan, state);
    if (tylan > support::MaxRangeOf<decltype(object.tylan)>::value) {
        throw "tylan too big!";
    }
    object.tylan = tylan;

    auto attrev = ResolveExpression(*state.result.psect.revision, state);
    if (attrev > support::MaxRangeOf<decltype(object.revision)>::value) {
        throw "attrev too big!";
    }
    object.revision = attrev;

    auto edition = ResolveExpression(*state.result.psect.edition, state);
    if (edition > support::MaxRangeOf<decltype(object.edition)>::value) {
        throw "edition too big!";
    }
    object.edition = edition;

    // TODO: this logic should be moved to ObjectFile implementations
//    if (state.counter.uninitialized_data > support::MaxRangeOf<decltype(header.StaticDataSize())>::value) {
//        throw "Uninitialized counter got way too big!";
//    }
//    header.StaticDataSize() = state.counter.uninitialized_data;
//
//    // Note: this impl not done. needs to write all fields after static data size!
//    assert(false);
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