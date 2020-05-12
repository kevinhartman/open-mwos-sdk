#include "Assembler.h"
#include "AssemblerTypes.h"
#include "AssemblerTarget.h"
#include "AssemblyState.h"
#include "ExpressionResolver.h"
#include "Numeric.h"
#include "ROFObjectFile.h"

#include <limits>
#include <vector>

namespace assembler {

extern bool HandlePseudoInstruction(const Entry &entry, AssemblyState &state);
extern bool HandleDirective(const Entry &entry, AssemblyState &state);

namespace {

void CreateExternalDefinitions(AssemblyState& state) {
    auto& extern_definitions = state.result.external_definitions;
    for (auto& elem : state.symbols) {
        auto& [label, symbol] = elem.second;

        if (label.is_global) {
            rof::ExternDefinition definition {};
            definition.Name() = label.name;
            definition.Type() = symbol.
            extern_definitions.emplace_back({})
        }
    }
}

void CreateHeader(uint16_t assembler_version, AssemblyState& state) {
    auto& header = state.result.header;
    header.Name() = state.psect.name;

    auto tylan = ResolveExpression(*state.psect.tylan, state);
    if (tylan > support::MaxRangeOf<decltype(header.TypeLanguage())>::value) {
        throw "tylan too big!";
    }
    header.TypeLanguage() = tylan;

    auto attrev = ResolveExpression(*state.psect.revision, state);
    if (attrev > support::MaxRangeOf<decltype(header.Revision())>::value) {
        throw "attrev too big!";
    }
    header.Revision() = attrev;

    // TODO: write non-zero if assembly fails? Why even produce ROF?
    header.AsmValid() = 0;
    header.AsmVersion() = assembler_version;

    // TODO: For now, just 0'd. not sure the format. Could be DNP3?
    header.AsmDate() = {0, 0, 0, 0, 0, 0};

    auto edition = ResolveExpression(*state.psect.edition, state);
    if (edition > support::MaxRangeOf<decltype(header.Edition())>::value) {
        throw "edition too big!";
    }
    header.Edition() = edition;

    if (state.counter.uninitialized_data > support::MaxRangeOf<decltype(header.StaticDataSize())>::value) {
        throw "Uninitialized counter got way too big!";
    }
    header.StaticDataSize() = state.counter.uninitialized_data;

    // TODO: not done. needs to write all fields after static data size!
    assert(false);
}

void CreateResult(AssemblyState& state) {

}

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

void Assembler::Process(const std::vector<Entry> &listing) {
    AssemblyState state {};

    for (auto& entry : listing) {
        if (state.found_program_end) {
            CreateResult(state);
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

    CreateResult(state);
}

}