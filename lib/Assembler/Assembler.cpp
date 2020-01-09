#include "Assembler.h"
#include "AssemblerTypes.h"
#include "AssemblerTarget.h"
#include "AssemblyState.h"

#include <vector>

namespace assembler {

namespace pseudoinst {
extern bool HandlePseudoInstruction(const Entry &entry, AssemblyState &state);
}

namespace directive {
extern bool HandleDirective(const Entry &entry, AssemblyState &state);
}

Assembler::Assembler(std::unique_ptr<AssemblerTarget> target) : target(std::move(target)) { }
Assembler::~Assembler() = default;

bool Assembler::HandlePseudoInstruction(const Entry& entry, AssemblyState& state) {
    return assembler::pseudoinst::HandlePseudoInstruction(entry, state);
}

bool Assembler::HandleDirective(const Entry& entry, AssemblyState& state) {
    return assembler::directive::HandleDirective(entry, state);
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