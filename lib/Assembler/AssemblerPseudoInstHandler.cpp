#include "AssemblerPseudoInstHandler.h"

#include <AssemblerTypes.h>
#include <Bitwise.h>
#include <Expression.h>

#include "AssemblyState.h"
#include "ExpressionResolver.h"
#include "Operation.h"

#include <iterator>
#include <unordered_map>
#include <ObjectFile.h>

#include <sstream>
#include <variant>
#include <AssemblyState.h>

namespace assembler {

// TODO:
//   - Operations which manipulate the various counters often include what are effectively
//     constant expressions (e.g. align 4+4). When these include external references, a
//     clear error message should inform the user that these expressions must be constexpr.

namespace {


/**
 * Ultra C Usage Guide:
 *
 * > Syntax
 * >   <label> dc.<size> <expression>{,<expression>}
 * > Description
 * >   dc generates sequences of one or more constants (initialized data) of various sizes
 * >   within the program. The argument is a list of one or more expressions or character
 * >   strings. If more than one expression or string is used, they are separated by commas.
 * >
 * >   The .<size> extension can be any of the below sizes:
 * >   - .b for bytes
 * >   - .sb for signed bytes
 * >   - .w for words (16-bit, default)
 * >   - .sw for signed words (16-bit)
 * >   - .l for longwords (32-bit)
 * >   - .sl for signed long words (32-bit)
 * >
 * >   The signed variants give the linker additional information about the nature of
 * >   a reference.
 * >
 * >   For example, if a signed word (.sw) external reference appeared in one psect and
 * >   another psect defined the value as 0xf000. The linker would complain about the value
 * >   0xf000 being too large to express in a 16-bit signed field. The linker would not have
 * >   complained if plain .w were used on the external reference.
 * >
 * >   Use the signed variants to ensure that large positive values are not accidentally
 * >   interpreted as negative values.
 * >
 * >   A dc used in a vsect creates an initialized data variable (read/write) in the process’
 * >   data area. The initialization value is stored in a special section of the object code
 * >   and is copied to the appropriate locations in the data area by the operating system.
 * >
 * >   A dc located outside a vsect creates read-only constants in the program area.
 * >   The program should not change these constants.
 * >
 * >   Character string constants can be any sequence of printable ASCII characters enclosed
 * >   in double quotes. For dc.w and dc.l, a string constant is padded with zeroes on the right
 * >   end if it does not fill the final word or long word. Therefore, dc.b is the most natural
 * >   format for strings.
 *
 * Notes:
 * Initialized data cannot exist in a VSect that is not nested within a Psect, since counters
 * do not work outside of psects.
 *
 * @tparam Size
 * @tparam IsSigned
 * @param operation
 * @param state
 */
template<size_t Size, bool IsSigned>
void Op_DC(std::unique_ptr<Operation> operation, AssemblyState& state) {
    constexpr auto context_err_msg = "must be in vsect within psect.";
    if (!state.in_psect)
        operation->Fail(OperationException::Code::NeedsPSectContext, context_err_msg);

    auto operands = operation->ParseOperands(Operation::SplitOnCommaRespectingStrings);

    auto& counter = state.GetInitDataCounter();
    auto& data_map = state.GetInitDataMap();

    for (std::size_t i = 0; i < operands.Count(); i++) {
        auto operand_str = operands.Get(i, "index " + std::to_string(i));

        if (operand_str->AsString().empty()) {
            operand_str->Fail("must be a valid expression or character string");
        }

        if (operand_str->AsString().front() == '"') {
            throw std::runtime_error("Character strings are not yet implemented.");
        } else {
            // handle expression
            auto value_operand = operands.GetExpression(i, "index " + std::to_string(i));
            // TODO: set operand size requirements on value_operand here
            //   so that when the expr is resolved in the second pass, an error can be thrown
            //   if the result fails to meet the context requirements.

            MemoryValue v {};
            v.data.u32 = 0;
            v.size = Size;
            v.is_signed = IsSigned;

            data_map[counter] = std::move(v);

            auto second_pass = std::make_unique<SecondPassAction>(
                [&field = data_map[counter]](AssemblyState& s, const ExpressionOperand& value_operand) {
                    ExpressionResolver resolver(s);

                    try {
                        auto value = value_operand.Resolve(resolver);
                        field.data.u32 = value;
                    } catch (OperandException& e) {
                        // Expression has external references.
                        field.expr_mappings = { ExpressionMapping {0,Size }};
                    }
                },
                std::move(value_operand)
            );

            state.DeferToSecondPass(std::move(second_pass));

            counter += Size;
        }
    }

}

/**
 * Ultra C Usage Guide:
 *
 * > Syntax
 * >   <label> ds.<size> <expr>
 * > Description
 * >   ds is used within a vsect to declare storage for uninitialized variables in the data area.
 * >   The .<size> extension can be:
 * >   - .b for bytes
 * >   - .sb for signed bytes
 * >   - .w for words (16-bit, default)
 * >   - .sw for words (16-bit)
 * >   - .l for longwords (32-bit)
 * >   - .sl for signed longwords (32-bit)
 * >
 * >  <expr> specifies the size of the variable in bytes, words, or longwords depending on the size
 * >  given for the ds extension. This value is added to the appropriate uninitialized data location
 * >  counter in order to update it.
 * >
 * >  When ds is used to declare variables, a label is usually specified which is assigned the
 * >  variable’s relative address. In OS-9 for 68K and OS-9, the address is not absolute.
 * >  Instead, indexed addressing modes are used to access variables. The actual relative
 * >  address is not assigned until the linker processes the ROF.
 *
 * Notes:
 * The <expr> field seems to support expressions with EQU and SET names, but not
 * data or code references.
 *
 * TODO:
 *   - EQU and SETs in <expr> names will only work if they were defined lexically
 *     before the current DS directive. This is correct behavior for Set but that's
 *     not clear in the case of EQU.
 *   - We correctly allow code and data references in the expression if they
 *     were defined earlier, but we need to check that they're subtracted from each other,
 *     since absolute values of counters are apparently disallowed.
 *   - It seems like DS is allowed outside of a psect.
 *     From Ultra C usage guide:
 *     > Although technically a vsect can appear outside the psect, the usefulness
 *     > of such a vsect is limited to defining the expected type of an external
 *     > symbol as a data area symbol because actual storage would not be assigned
 *     > to it by the linker.
 *
 * @tparam Size
 * @tparam IsSigned
 * @param operation
 * @param state
 */
template<size_t Size, bool IsSigned>
void Op_DS(std::unique_ptr<Operation> operation, AssemblyState& state) {
    constexpr auto context_err_msg = "must be in vsect within psect.";
    if (!state.in_psect)
        operation->Fail(OperationException::Code::NeedsPSectContext, context_err_msg);
    if (!state.in_vsect)
        operation->Fail(OperationException::Code::NeedsVSectContext, context_err_msg);

    auto operands = operation->ParseOperands();
    auto count_operand = operands.GetExpression(0, "count");

    // If a label was provided inline, add it to pending labels.
    auto label_opt = operation->GetEntry().label;
    if (label_opt) {
        state.pending_labels.emplace_back(label_opt.value());
    }

    ExpressionResolver resolver(state);

    // We need to resolve the size (count) expression *now*, since we must know by how much to increment data counter.
    auto count = count_operand->Resolve(resolver);
    if (count < 1) {
        // count cannot be < 1, since this would result in a symbol of size 0!
        count_operand->Fail("must be > 0");
    }

    auto increment = count * Size;

    auto& counter = state.in_remote_vsect
        ? state.result->counter.remote_uninitialized_data
        : state.result->counter.uninitialized_data;

    // Automatically align to even boundary for word and long (according to documentation).
    if constexpr (Size > 1) {
        counter = support::RoundToNextPow2Multiple(counter, 2);
    }

    // Consume pending labels here and reinitialize them to empty.
    std::vector<Label> labels {};
    std::swap(state.pending_labels, labels);

    for (auto& label : labels) {
        if (state.GetSymbol(label.name)) {
            operation->Fail(OperationException::Code::DuplicateSymbol,
                "label must not be previously defined");
        }

        state.UpdateSymbol(label, object::SymbolInfo {
            state.in_remote_vsect
                ? object::SymbolInfo::Type::RemoteUninitData
                : object::SymbolInfo::Type::UninitData,
            label.is_global,
            counter
        });
    }

    // Advance counter now that labels are mapped.
    counter += increment;
}

/**
 * Ultra C Usage Guide:
 *
 * > Syntax
 * >   align <alignment boundary>
 * > Description
 * >   align aligns the next generated code or next assigned data offset on
 * >   some byte boundary in memory. If the current value of the instruction
 * >   counter is not aligned to the alignment boundary, sufficient zero bytes
 * >   are inserted in the object code to force the desired alignment.
 * >
*  > <alignment boundary> specifies the alignment to use. If <alignment boundary>
 * > is not specified, align uses an alignment boundary of two bytes. <alignment boundary>
 * > must be a power of two.
 * >
 * > If align is specified in a vsect, the assembler aligns both the initialized
 * > and uninitialized location counters.
 * >
 * > align is generally used after odd length constant tables, character strings,
 * > or character data are embedded in the object code.
 *
 * Notes:
 * Align is allowed to accept expressions that include EQU and SET names, but not
 * code or data references.
 *
 * TODO:
 *   - Currently, this implementation only allows numeric constant expressions,
 *     but it should support any expression as long as it does not include
 *     data and code reference names.
 *
 * @param operation
 * @param state
 */
void Op_Align(std::unique_ptr<Operation> operation, AssemblyState& state) {
    if (!state.in_psect) {
        operation->Fail(OperationException::Code::NeedsPSectContext, "must be inside psect");
    }

    uint32_t alignment = 2; // default

    auto operands = operation->ParseOperands();
    if (operands.Count() > 0) {
        auto operand_alignment = operands.GetExpression(0, "alignment");

        ExpressionResolver resolver(state);
        alignment = operand_alignment->Resolve(resolver);

        if (!support::IsPow2(alignment)) {
            operand_alignment->Fail("must be power of 2");
        }
    }

    auto align = [&alignment](auto &counter) {
        counter = support::RoundToNextPow2Multiple(counter, alignment);
    };

    auto& counter = state.result->counter;
    if (state.in_vsect) {
        // Align data counters
        align(state.in_remote_vsect ? counter.remote_initialized_data : counter.initialized_data);
        align(state.in_remote_vsect ? counter.remote_uninitialized_data : counter.uninitialized_data);
    } else {
        // Align code section.
        align(counter.code);
    }
}

void Op_Unimplemented(std::unique_ptr<Operation> operation, AssemblyState& state) {
    throw "pseudo instruction is unimplemented: " + operation->GetEntry().operation.value_or("");
}

typedef void (*PseudoInstFunc)(std::unique_ptr<Operation>, AssemblyState&);
std::unordered_map<std::string, PseudoInstFunc> pseudo_instructions = {
    { "align", Op_Align },
    { "com",   Op_Unimplemented },

    { "dc.b",  Op_DC<1, false> },
    { "dc.sb", Op_DC<1, true> },
    { "dc.w",  Op_DC<2, false> },
    { "dc.sw", Op_DC<2, true> },
    { "dc.l",  Op_DC<4, false> },
    { "dc.sl", Op_DC<4, true> },

    { "do.b",  Op_Unimplemented },
    { "do.sb", Op_Unimplemented },
    { "do.w",  Op_Unimplemented },
    { "do.sw", Op_Unimplemented },
    { "do.l",  Op_Unimplemented },
    { "do.sl", Op_Unimplemented },

    { "ds.b",  Op_DS<1, false> },
    { "ds.sb", Op_DS<1, true> },
    { "ds.w",  Op_DS<2, false> },
    { "ds.sw", Op_DS<2, true> },
    { "ds.l",  Op_DS<4, false> },
    { "ds.sl", Op_DS<4, true> },

    { "dz.b",  Op_Unimplemented },
    { "dz.sb", Op_Unimplemented },
    { "dz.w",  Op_Unimplemented },
    { "dz.sw", Op_Unimplemented },
    { "dz.l",  Op_Unimplemented },
    { "dz.sl", Op_Unimplemented },

    { "lo.b",  Op_Unimplemented },
    { "lo.sb", Op_Unimplemented },
    { "lo.w",  Op_Unimplemented },
    { "lo.sw", Op_Unimplemented },
    { "lo.l",  Op_Unimplemented },
    { "lo.sl", Op_Unimplemented },

    { "org",   Op_Unimplemented },
    { "tcall", Op_Unimplemented }
};
}

AssemblerPseudoInstHandler::~AssemblerPseudoInstHandler() = default;

bool AssemblerPseudoInstHandler::Handle(const Entry& entry, AssemblyState& state) {
    assert(entry.operation);

    auto operation = std::make_unique<Operation>(entry);

    auto handler_kv = pseudo_instructions.find(entry.operation.value());
    if (handler_kv != pseudo_instructions.end()) {
        handler_kv->second(std::move(operation), state);
        return true;
    }

    return false;
}
}