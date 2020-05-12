#include <catch2/catch.hpp>
#include <capstone/capstone.h>

#include "ComparisonHelpers.h"
#include "PrinterHelpers.h"

#include "AssemblerTypes.h"
#include "MipsAssemblerTarget.h"

#include "ExpressionLexer.h"
#include "ExpressionParser.h"

#include "Endian.h"

#include <limits>
#include <optional>
#include <vector>
#include <iterator>

namespace assembler {
    struct InstructionToExpected {
        Entry input_instruction;
        std::string expected_capstone_str;
        std::vector<object::ExpressionMapping> expected_expr_mappings;
    };

    auto MakeEntry(std::string operation, std::optional<std::string> operand_str) {
        return Entry { std::nullopt, operation, operand_str, std::nullopt };
    }

    template <typename ...T>
    auto MakeExpressionMappings(T&&... mappings) {
        return std::vector<object::ExpressionMapping> { std::forward<T>(mappings)... };
    }

    object::ExpressionMapping MakeExpressionMapping(size_t offset, size_t bit_count, std::string expression_str) {
        ExpressionLexer lexer(expression_str);
        ExpressionParser parser(lexer);

        return object::ExpressionMapping { offset, bit_count, parser.Parse() };
    }

    std::optional<std::string> DecodeWithCapstone(uint32_t instruction) {
        std::optional<std::string> result {};
        csh handle;
        cs_insn *insn;
        size_t count;

        // TODO: use host endian (default is little for CS_MODE)
        if (cs_open(CS_ARCH_MIPS, static_cast<cs_mode>(CS_MODE_MIPS32 | CS_MODE_MIPS2 | CS_MODE_BIG_ENDIAN), &handle) != CS_ERR_OK)
            throw "Couldn't open Capstone MIPS decoder.";

        count = cs_disasm(handle, reinterpret_cast<uint8_t*>(&instruction), sizeof(instruction), 0x1000, 0, &insn);
        if (count > 0) {
            if (count > 1) {
                throw "Unexpectedly decoded > 1 instruction.";
            }

            std::string operands(insn[0].op_str);
            result = std::string(insn[0].mnemonic) + " " + operands;
            cs_free(insn, count);
        }

        cs_close(&handle);

        return result;
    }

    SCENARIO("Mips instructions are properly encoded", "[amips][Assembler]") {
        GIVEN("each MIPS assembler instruction") {
            auto pair = GENERATE(values<InstructionToExpected>({
                { MakeEntry("add", "at,v0,v1"), "add $at, $v0, $v1", {} },
                { MakeEntry("addi", "k1,sp,-32"), "addi $k1, $sp, 0", MakeExpressionMappings(MakeExpressionMapping(0, 16, "-32")) },
                { MakeEntry("addu", "t1,t1,a0"), "addu $t1, $t1, $a0", {}},
                { MakeEntry("sb", "zero,-4(t0)"), "sb $zero, ($t0)", MakeExpressionMappings(MakeExpressionMapping(0, 16, "-4")) },
                { MakeEntry("jal", "0xFFFFFF"), "jal 0", MakeExpressionMappings(MakeExpressionMapping(0, 26, "0xFFFFFF"))},
                { MakeEntry("srl", "k0,$5,$1D"), "srl $k0, $a1, 0", MakeExpressionMappings(MakeExpressionMapping(6, 5, "$1D")) },
                { MakeEntry("jalr", "t4"), "jalr $t4", {} },
                { MakeEntry("jalr", "$31,t4"), "jalr $t4", {} },
                { MakeEntry("jalr", "$30,t4"), "jalr $fp, $t4", {} },
            }));

            WHEN("the expression is parsed") {
                MipsAssemblerTarget target(support::Endian::big);

                THEN("the expression tree is correct") {
                    auto encoded = target.EmitInstruction(pair.input_instruction);
                    REQUIRE(encoded.size == 4);

                    auto capstone_result = DecodeWithCapstone(static_cast<uint32_t>(encoded.data.u32));
                    REQUIRE(capstone_result);
                    REQUIRE(capstone_result.value() == pair.expected_capstone_str);
                    REQUIRE_THAT(encoded.expr_mappings, Catch::UnorderedEquals(pair.expected_expr_mappings));
                }
            }
        }
    }
}