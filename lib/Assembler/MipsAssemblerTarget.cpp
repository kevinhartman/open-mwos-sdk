#include "MipsAssemblerTarget.h"
#include "AssemblerTypes.h"
#include "StringUtil.h"
#include "ExpressionLexer.h"
#include "ExpressionParser.h"

#include <string>
#include <regex>
#include <vector>
#include <tuple>

namespace assembler {

MipsAssemblerTarget::MipsAssemblerTarget() = default;
MipsAssemblerTarget::~MipsAssemblerTarget() = default;

uint32_t ParseRegister(std::string register_str) {
    auto name_to_reg = std::vector<std::regex> {
        std::regex(R"((\$0|zero))"),
        std::regex(R"((\$1|AT))"),
        std::regex(R"((\$2|v0))"),
        std::regex(R"((\$3|v1))"),
        std::regex(R"((\$4|a0))"),
        std::regex(R"((\$5|a1))"),
        std::regex(R"((\$6|a2))"),
        std::regex(R"((\$7|a3))"),
        std::regex(R"((\$8|t0))"),
        std::regex(R"((\$9|t1))"),
        std::regex(R"((\$10|t2))"),
        std::regex(R"((\$11|t3))"),
        std::regex(R"((\$12|t4))"),
        std::regex(R"((\$13|t5))"),
        std::regex(R"((\$14|t6))"),
        std::regex(R"((\$15|t7))"),
        std::regex(R"((\$16|s0))"),
        std::regex(R"((\$17|s1))"),
        std::regex(R"((\$18|s2))"),
        std::regex(R"((\$19|s3))"),
        std::regex(R"((\$20|s4))"),
        std::regex(R"((\$21|s5))"),
        std::regex(R"((\$22|s6))"),
        std::regex(R"((\$23|s7))"),
        std::regex(R"((\$24|t8))"),
        std::regex(R"((\$25|t9))"),
        std::regex(R"((\$26|k0))"),
        std::regex(R"((\$27|k1))"),
        std::regex(R"((\$28|gp))"),
        std::regex(R"((\$29|sp))"),
        std::regex(R"((\$30|cp))"),
        std::regex(R"((\$31|ra))")
    };

    for (u_int8_t reg_id; reg_id < 32; reg_id++) {
        if (std::regex_match(register_str, name_to_reg.at(reg_id))) {
            return reg_id;
        }
    }

    throw "invalid reg name";
}

std::unique_ptr<Expression> ParseExpression(const std::string& expr_str) {
    auto lexer = ExpressionLexer(expr_str);
    auto parser = ExpressionParser(lexer);

    return parser.Parse();
}

struct RT : std::optional<std::string> { using optional::optional; };
struct RS : std::optional<std::string> { using optional::optional; };
struct RD : std::optional<std::string> { using optional::optional; };
struct Immediate : std::optional<std::string> { using optional::optional; };
struct Shift : std::optional<std::string> { using optional::optional; };
struct Target : std::optional<std::string> { using optional::optional; };

template<typename Arg1, typename Arg2, typename Arg3>
std::tuple<RS, RT, RD, Shift> RTypeTupleSyntax(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));
    auto tup = std::make_tuple<RS, RT, RD, Shift>(std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = operands[1];
    std::get<Arg3>(tup) = operands[2];

    return tup;
}

template<typename Arg1, typename Arg2>
std::tuple<RS, RT, RD, Shift> RTypeTupleSyntax(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));
    auto tup = std::make_tuple<RS, RT, RD, Shift>(std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = operands[1];

    return tup;
}

template<typename Arg1>
std::tuple<RS, RT, RD, Shift> RTypeTupleSyntax(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));
    auto tup = std::make_tuple<RS, RT, RD, Shift>(std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    std::get<Arg1>(tup) = operands[0];

    return tup;
}

std::tuple<RS, RT, RD, Shift> RTypeNoArgs(std::string operand_str) {
    return std::make_tuple<RS, RT, RD, Shift>(std::nullopt, std::nullopt, std::nullopt, std::nullopt);
}

template<typename Arg1, typename Arg2, typename Arg3>
std::tuple<RS, RT, Immediate> ITypeTupleSyntax(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));

    auto tup = std::make_tuple<RS, RT, Immediate>(std::nullopt, std::nullopt, std::nullopt);
    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = operands[1];
    std::get<Arg3>(tup) = operands[2];

    return tup;
}

template<typename Arg1, typename Arg2, typename Arg3>
std::tuple<RS, RT, Immediate> ITypeOffsetSyntax(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));

    std::smatch matches;
    std::regex_search(operands[1], matches, std::regex(R"((.*)\((.*)\))"));

    auto tup = std::make_tuple<RS, RT, Immediate>(std::nullopt, std::nullopt, std::nullopt);
    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = matches[1].str();
    std::get<Arg3>(tup) = matches[2].str();

    return tup;
}

template<typename Arg1, typename Arg2>
std::tuple<RS, RT, Immediate> ITypeTupleSyntax(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));
    auto tup = std::make_tuple<RS, RT, Immediate>(std::nullopt, std::nullopt, std::nullopt);

    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = operands[1];

    return tup;
}

template<typename Arg1>
std::tuple<Target> JTypeTupleSyntax(std::string operand_str) {
    auto tup = std::make_tuple<Target>(std::nullopt);
    std::get<Arg1>(tup) = operand_str;

    return tup;
}

typedef std::tuple<RS, RT, RD, Shift> (*RTypeSyntaxFunc)(std::string);

template <uint32_t OpCode, uint32_t RS, uint32_t RT, uint32_t RD, uint32_t Shift, uint32_t FuncCode, RTypeSyntaxFunc Syntax>
Instruction ParseRType(const Entry& entry) {
    constexpr uint32_t Data = (OpCode << 26U) | (RS << 21U) | (RT << 16U) | (RD << 11U) | (Shift << 6U) | FuncCode;

    Instruction instruction {};
    instruction.data = Data;
    instruction.size = 4;

    auto operands = Syntax(entry.operands.value());

    if (std::get<assembler::RS>(operands)) {
        assert(RS == 0);
        instruction.data |= ParseRegister(std::get<assembler::RS>(operands).value()) << 21U;
    }

    if (std::get<assembler::RT>(operands)) {
        assert(RT == 0);
        instruction.data |= ParseRegister(std::get<assembler::RT>(operands).value()) << 16U;
    }

    if (std::get<assembler::RD>(operands)) {
        assert(RD == 0);
        instruction.data |= ParseRegister(std::get<assembler::RD>(operands).value()) << 11U;
    }

    if (std::get<assembler::Shift>(operands)) {
        assert(Shift == 0);
        instruction.expr_mappings.emplace_back(
            ExpressionMapping{ 6, 5, ParseExpression(std::get<assembler::Shift>(operands).value())});
    }

    return instruction;
}

typedef std::tuple<RS, RT, Immediate> (*ITypeSyntaxFunc)(std::string);

template <uint32_t OpCode, uint32_t RS, uint32_t RT, uint32_t Immediate, ITypeSyntaxFunc Syntax>
Instruction ParseIType(const Entry& entry) {
    constexpr uint32_t Data = (OpCode << 26U) | (RS << 21U) | (RT << 16U) | Immediate;

    Instruction instruction {};
    instruction.data = Data;
    instruction.size = 4;

    auto operands = Syntax(entry.operands.value());

    if (std::get<assembler::RS>(operands)) {
        assert(RS == 0);
        instruction.data = ParseRegister(std::get<assembler::RS>(operands).value()) << 21U;
    }

    if (std::get<assembler::RT>(operands)) {
        assert(RT == 0);
        instruction.data = ParseRegister(std::get<assembler::RT>(operands).value()) << 16U;
    }

    if (std::get<assembler::Immediate>(operands)) {
        assert(Immediate == 0);
        instruction.expr_mappings.emplace_back(
            ExpressionMapping{ 0, 16, ParseExpression(std::get<assembler::Immediate>(operands).value()) });
    }

    return instruction;
}

typedef std::tuple<Target> (*JTypeSyntaxFunc)(std::string);

template <uint32_t OpCode, uint32_t Target, JTypeSyntaxFunc Syntax>
Instruction ParseJType(const Entry& entry) {
    constexpr uint32_t Data = (OpCode << 26U) | Target;

    Instruction instruction {};
    instruction.data = Data;
    instruction.size = 4;

    auto operands = Syntax(entry.operands.value());
    if (std::get<assembler::Target>(operands)) {
        assert(Target == 0);
        instruction.expr_mappings.emplace_back(
            ExpressionMapping{ 0, 26, ParseExpression(std::get<assembler::Target>(operands).value()) });
    }

    return instruction;
}

Instruction ParseJALR(const Entry& entry) {
    try {
        ParseRType<0b000000, 0, 0b00000, 0, 0b000000, 0b001001, RTypeTupleSyntax<RD, RS>>(entry);
    } catch (...) {
        // Try to parse as single register. If it works (it's RS), inject default $31 for RD.
        ParseRegister(entry.operands.value_or(""));
        ParseRType<0b000000, 0, 0b00000, 0, 0b000000, 0b001001, RTypeTupleSyntax<RD, RS>>(
            Entry {
                entry.label,
                entry.operation,
                "$31," + entry.operands.value(),
                entry.comment
            }
        );
    }
}

Instruction ThrowUnimplemented(const Entry& entry) {
    throw "unimplemented";
}

Instruction ThrowInvalidCoprocessor(const Entry& entry) {
    throw "Instruction not supported by coprocessor: " + entry.operation.value();
}

typedef Instruction (*ParseFunc)(const Entry&);
std::map<std::string, ParseFunc> instructions_fn = {
    { "add",    ParseRType<0b000000, 0, 0, 0, 0, 0b100000, RTypeTupleSyntax<RD, RS, RT>> },
    { "addi",   ParseIType<0b001000, 0, 0, 0, ITypeTupleSyntax<RT, RS, Immediate>> },
    { "addiu",  ParseIType<0b001001, 0, 0, 0, ITypeTupleSyntax<RT, RS, Immediate>> },
    { "addu",   ParseRType<0b000000, 0, 0, 0, 0, 0b100001, RTypeTupleSyntax<RD, RS, RT>> },
    { "and",    ParseRType<0b000000, 0, 0, 0, 0, 0b100100, RTypeTupleSyntax<RD, RS, RT>> },
    { "andi",   ParseIType<0b001100, 0, 0, 0, ITypeTupleSyntax<RT, RS, Immediate>> },
    { "beq",    ParseIType<0b000100, 0, 0, 0, ITypeTupleSyntax<RS, RT, Immediate>> },
    { "bgez",   ParseIType<0b000001, 0, 0b00001, 0, ITypeTupleSyntax<RS, Immediate>> },
    { "bgezal", ParseIType<0b000001, 0, 0b10001, 0, ITypeTupleSyntax<RS, Immediate>> },
    { "bgtz",   ParseIType<0b000111, 0, 0b00000, 0, ITypeTupleSyntax<RS, Immediate>> },
    { "blez",   ParseIType<0b000110, 0, 0b00000, 0, ITypeTupleSyntax<RS, Immediate>> },
    { "bltz",   ParseIType<0b000001, 0, 0b00000, 0, ITypeTupleSyntax<RS, Immediate>> },
    { "bltzal", ParseIType<0b000001, 0, 0b10000, 0, ITypeTupleSyntax<RS, Immediate>> },
    { "bne",    ParseIType<0b000101, 0, 0, 0, ITypeTupleSyntax<RS, RT, Immediate>> },
    { "break",  ParseRType<0b000000, 0, 0, 0, 0, 0b001101, RTypeNoArgs> },
    { "cfc0",   ThrowInvalidCoprocessor },
    { "cfc1",   ParseRType<0b010001, 0b00010, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "cfc2",   ParseRType<0b010010, 0b00010, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "cfc3",   ParseRType<0b010011, 0b00010, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "cop0",   ThrowUnimplemented },
    { "cop1",   ThrowUnimplemented },
    { "cop2",   ThrowUnimplemented },
    { "cop3",   ThrowUnimplemented },
    { "ctc0",   ThrowInvalidCoprocessor },
    { "ctc1",   ParseRType<0b010001, 0b00110, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "ctc2",   ParseRType<0b010010, 0b00110, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "ctc3",   ParseRType<0b010011, 0b00110, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "div",    ParseRType<0b000000, 0, 0, 0b00000, 0b000000, 0b011010, RTypeTupleSyntax<RS, RT>> },
    { "divu",   ParseRType<0b000000, 0, 0, 0b00000, 0b000000, 0b011011, RTypeTupleSyntax<RS, RT>> },
    { "j",      ParseJType<0b000010, 0, JTypeTupleSyntax<Target>> },
    { "jal",    ParseJType<0b000011, 0, JTypeTupleSyntax<Target>> },
    { "jalr",   ParseJALR /* special handling due to optional RD (defaults to $31)*/ },
    { "jr",     ParseRType<0b000000, 0, 0b00000, 0b00000, 0b000000, 0b001000, RTypeTupleSyntax<RS>> },
    { "lb",     ParseIType<0b100000, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lbu",    ParseIType<0b100100, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lh",     ParseIType<0b100001, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lhu",    ParseIType<0b100101, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lui",    ParseIType<0b001111, 0b00000, 0, 0, ITypeTupleSyntax<RT, Immediate>> },
    { "lw",     ParseIType<0b100011, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lwc0",   ThrowInvalidCoprocessor },
    { "lwc1",   ParseIType<0b110001, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lwc2",   ParseIType<0b110010, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lwc3",   ParseIType<0b110011, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lwl",    ParseIType<0b100010, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "lwr",    ParseIType<0b100110, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "mfc0",   ParseRType<0b010000, 0b00000, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "mfc1",   ParseRType<0b010001, 0b00000, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "mfc2",   ParseRType<0b010010, 0b00000, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "mfc3",   ParseRType<0b010011, 0b00000, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "mfhi",   ParseRType<0b000000, 0b00000, 0b00000, 0, 0b00000, 0b010000, RTypeTupleSyntax<RD>> },
    { "mflo",   ParseRType<0b000000, 0b00000, 0b00000, 0, 0b00000, 0b010010, RTypeTupleSyntax<RD>> },
    { "mtc0",   ParseRType<0b010000, 0b00100, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "mtc1",   ParseRType<0b010001, 0b00100, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "mtc2",   ParseRType<0b010010, 0b00100, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "mtc3",   ParseRType<0b010011, 0b00100, 0, 0, 0b00000, 0b000000, RTypeTupleSyntax<RT, RD>> },
    { "mthi",   ParseRType<0b000000, 0, 0b00000, 0b00000, 0b00000, 0b010001, RTypeTupleSyntax<RS>> },
    { "mtlo",   ParseRType<0b000000, 0, 0b00000, 0b00000, 0b00000, 0b010011, RTypeTupleSyntax<RS>> },
    { "mult",   ParseRType<0b000000, 0, 0, 0b00000, 0b00000, 0b011000, RTypeTupleSyntax<RS, RT>> },
    { "multu",  ParseRType<0b000000, 0, 0, 0b00000, 0b00000, 0b011001, RTypeTupleSyntax<RS, RT>> },
    { "nor",    ParseRType<0b000000, 0, 0, 0, 0b00000, 0b100111, RTypeTupleSyntax<RD, RS, RT>> },
    { "or",     ParseRType<0b000000, 0, 0, 0, 0b00000, 0b100101, RTypeTupleSyntax<RD, RS, RT>> },
    { "ori",    ParseIType<0b001101, 0, 0, 0, ITypeTupleSyntax<RT, RS, Immediate>> },
    { "sb",     ParseIType<0b101000, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "sh",     ParseIType<0b101001, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "sll",    ParseRType<0b000000, 0b00000, 0, 0, 0, 0b000000, RTypeTupleSyntax<RD, RT, Shift>> },
    { "sllv",   ParseRType<0b000000, 0, 0, 0, 0b00000, 0b000100, RTypeTupleSyntax<RD, RT, RS>> },
    { "slt",    ParseRType<0b000000, 0, 0, 0, 0b00000, 0b101010, RTypeTupleSyntax<RD, RS, RT>> },
    { "slti",   ParseIType<0b001010, 0, 0, 0, ITypeTupleSyntax<RT, RS, Immediate>> },
    { "sltiu",  ParseIType<0b001011, 0, 0, 0, ITypeTupleSyntax<RT, RS, Immediate>> },
    { "sltu",   ParseRType<0b000000, 0, 0, 0, 0b00000, 0b101011, RTypeTupleSyntax<RD, RS, RT>> },
    { "sra",    ParseRType<0b000000, 0b00000, 0, 0, 0, 0b000011, RTypeTupleSyntax<RD, RT, Shift>> },
    { "srav",   ParseRType<0b000000, 0, 0, 0, 0b00000, 0b000111, RTypeTupleSyntax<RD, RT, RS>> },
    { "srl",    ParseRType<0b000000, 0b00000, 0, 0, 0, 0b000010, RTypeTupleSyntax<RD, RT, Shift>> },
    { "srlv",   ParseRType<0b000000, 0, 0, 0, 0b00000, 0b000110, RTypeTupleSyntax<RD, RT, RS>> },
    { "sub",    ParseRType<0b000000, 0, 0, 0, 0b00000, 0b100010, RTypeTupleSyntax<RD, RS, RT>> },
    { "subu",   ParseRType<0b000000, 0, 0, 0, 0b00000, 0b100011, RTypeTupleSyntax<RD, RS, RT>> },
    { "sw",     ParseIType<0b101011, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "swc0",   ThrowInvalidCoprocessor },
    { "swc1",   ParseIType<0b111001, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "swc2",   ParseIType<0b111010, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "swc3",   ParseIType<0b111011, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "swl",    ParseIType<0b101010, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "swr",    ParseIType<0b101110, 0, 0, 0, ITypeOffsetSyntax<RT, Immediate, RS>> },
    { "syscall",ParseRType<0b000000, 0b00000, 0b00000, 0b00000, 0b00000, 0b001100, RTypeNoArgs> },
    { "xor",    ParseRType<0b000000, 0, 0, 0, 0b00000, 0b100110, RTypeTupleSyntax<RD, RS, RT>> },
    { "xori",   ParseIType<0b001110, 0, 0, 0, ITypeTupleSyntax<RT, RS, Immediate>> },

};

Instruction MipsAssemblerTarget::EmitInstruction(const Entry& entry) {
    return instructions_fn[entry.operation.value()](entry);
}

}