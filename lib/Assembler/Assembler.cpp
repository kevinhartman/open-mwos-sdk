#include "Assembler.h"
#include "AssemblerDirectiveHandler.h"
#include "AssemblerPseudoInstHandler.h"
#include "AssemblerTypes.h"
#include "AssemblerTarget.h"
#include "AssemblyState.h"
#include "ExpressionResolver.h"
#include "ObjectFile.h"

#include <chrono>
#include <vector>

namespace assembler {

void Assembler::CreateResult(AssemblyState& state) {
    // Invoke second pass.
    for (auto& action : state.second_pass_queue2) {
        (*action)(state);
    }

    // Set static properties.
    using clock = std::chrono::system_clock;
    auto now = clock::now();
    state.result->assembly_time = clock::to_time_t(now);

    // Set assembler info
    state.result->assembler_version = assembler_version;
}

Assembler::Assembler(uint16_t assembler_version, std::unique_ptr<AssemblerTarget> target)
    : assembler_version(assembler_version), target(std::move(target))
{
    op_handlers.emplace_back(std::make_unique<AssemblerDirectiveHandler>());
    op_handlers.emplace_back(std::make_unique<AssemblerPseudoInstHandler>());
}

Assembler::~Assembler() = default;

std::unique_ptr<object::ObjectFile> Assembler::Process(const std::vector<Entry> &listing) {
    AssemblyState state {};

    // Set target CPU ID and endianness on object file.
    target->SetTargetSpecificProperties(*state.result);

    for (auto& entry : listing) {
        if (state.found_program_end) {
            break;
        }

        if (entry.label) {
            // Remember the label so it can be mapped to the next appropriate counter value.
            state.pending_labels.insert(entry.label.value());
        }

        if (entry.operation) {
            auto try_handle = [&entry, &state](auto& handler) {
                return handler->Handle(entry, state);
            };

            if (!std::any_of(op_handlers.begin(), op_handlers.end(), try_handle)) {
                // This must be a CPU instruction.

                // TODO: maybe it's cleaner to just register CPU instruction handling
                //       as a separate handler (perhaps CPUInstructionHandler, which itself
                //       would take the AssemblerTarget instance instead of that being provided
                //       to the Assembler class).

                // TODO: check this conditional it looks odd
                if (!(state.in_psect && !state.in_vsect)) {
                    throw "target instruction must be inside the psect";
                }

                target->GetOperationHandler()->Handle(entry, state);
            }
        }
    }

    CreateResult(state);
    return std::move(state.result);
}

}