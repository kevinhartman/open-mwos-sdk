#include "MipsAssemblerTarget.h"
#include "AssemblerTypes.h"
#include "StringUtil.h"
#include "ExpressionLexer.h"
#include "ExpressionParser.h"

#include <string>
#include <regex>
#include <vector>
#include <tuple>
#include <AssemblerTarget.h>
#include <unordered_map>

namespace assembler {

using expression::Expression;

namespace {

uint32_t ParseRegister(std::string register_str) {
    auto name_to_reg = std::vector<std::regex> {
        std::regex(R"((\$0|zero))"),
        std::regex(R"((\$1|at))"),
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
        std::regex(R"((\$30|cp|fp))"),
        std::regex(R"((\$31|ra))")
    };

    for (u_int8_t reg_id = 0; reg_id < 32; reg_id++) {
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
std::tuple<RS, RT, RD, Shift> RTypeTuple(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));
    auto tup = std::make_tuple<RS, RT, RD, Shift>(std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = operands[1];
    std::get<Arg3>(tup) = operands[2];

    return tup;
}

template<typename Arg1, typename Arg2>
std::tuple<RS, RT, RD, Shift> RTypeTuple(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));
    auto tup = std::make_tuple<RS, RT, RD, Shift>(std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = operands[1];

    return tup;
}

template<typename Arg1>
std::tuple<RS, RT, RD, Shift> RTypeTuple(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));
    auto tup = std::make_tuple<RS, RT, RD, Shift>(std::nullopt, std::nullopt, std::nullopt, std::nullopt);

    std::get<Arg1>(tup) = operands[0];

    return tup;
}

std::tuple<RS, RT, RD, Shift> RTypeNoArgs(std::string operand_str) {
    return std::make_tuple<RS, RT, RD, Shift>(std::nullopt, std::nullopt, std::nullopt, std::nullopt);
}

template<typename Arg1, typename Arg2, typename Arg3>
std::tuple<RS, RT, Immediate> ITypeTuple(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));

    auto tup = std::make_tuple<RS, RT, Immediate>(std::nullopt, std::nullopt, std::nullopt);
    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = operands[1];
    std::get<Arg3>(tup) = operands[2];

    return tup;
}

template<typename Arg1, typename Arg2, typename Arg3>
std::tuple<RS, RT, Immediate> ITypeOffset(std::string operand_str) {
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
std::tuple<RS, RT, Immediate> ITypeTuple(std::string operand_str) {
    auto operands = Split(operand_str, std::regex(","));
    auto tup = std::make_tuple<RS, RT, Immediate>(std::nullopt, std::nullopt, std::nullopt);

    std::get<Arg1>(tup) = operands[0];
    std::get<Arg2>(tup) = operands[1];

    return tup;
}

template<typename Arg1>
std::tuple<Target> JTypeTuple(std::string operand_str) {
    auto tup = std::make_tuple<Target>(std::nullopt);
    std::get<Arg1>(tup) = operand_str;

    return tup;
}

//typedef uint32_t Field;
constexpr uint32_t Arg = std::numeric_limits<uint32_t>::max();
constexpr bool IsArgSentinel(uint32_t field) {
    return field == Arg;
}

typedef std::tuple<RS, RT, RD, Shift> (*RTypeSyntaxFunc)(std::string);

template <uint32_t OpCode, uint32_t RS, uint32_t RT, uint32_t RD, uint32_t Shift, uint32_t FuncCode, RTypeSyntaxFunc Syntax>
object::Instruction RType(const Entry& entry) {
    object::Instruction instruction {};
    instruction.data.u32 = OpCode << 26U | FuncCode;
    instruction.size = 4;

    auto operands = Syntax(entry.operands.value_or(""));

    if constexpr (IsArgSentinel(RS)) {
        instruction.data.u32 |= ParseRegister(std::get<assembler::RS>(operands).value()) << 21U;
    } else {
        instruction.data.u32 |= RS << 21U;
    }

    if constexpr (IsArgSentinel(RT)) {
        instruction.data.u32 |= ParseRegister(std::get<assembler::RT>(operands).value()) << 16U;
    } else {
        instruction.data.u32 |= RT << 16U;
    }

    if constexpr (IsArgSentinel(RD)) {
        instruction.data.u32 |= ParseRegister(std::get<assembler::RD>(operands).value()) << 11U;
    } else {
        instruction.data.u32 |= RD << 11U;
    }

    if constexpr (IsArgSentinel(Shift)) {
        instruction.expr_mappings.emplace_back(
            object::ExpressionMapping{ 6, 5, ParseExpression(std::get<assembler::Shift>(operands).value())});
    } else {
        instruction.data.u32 |= Shift << 6U;
    }

    return instruction;
}

typedef std::tuple<RS, RT, Immediate> (*ITypeSyntaxFunc)(std::string);

template <uint32_t OpCode, uint32_t RS, uint32_t RT, uint32_t Immediate, ITypeSyntaxFunc Syntax>
object::Instruction IType(const Entry& entry) {
    object::Instruction instruction {};
    instruction.data.u32 = OpCode << 26U;
    instruction.size = 4;

    auto operands = Syntax(entry.operands.value_or(""));

    if constexpr (IsArgSentinel(RS)) {
        instruction.data.u32 |= ParseRegister(std::get<assembler::RS>(operands).value()) << 21U;
    } else {
        instruction.data.u32 |= RS << 21U;
    }

    if constexpr (IsArgSentinel(RT)) {
        instruction.data.u32 |= ParseRegister(std::get<assembler::RT>(operands).value()) << 16U;
    } else {
        instruction.data.u32 |= RT << 16U;
    }

    if constexpr (IsArgSentinel(Immediate)) {
        instruction.expr_mappings.emplace_back(
            object::ExpressionMapping{ 0, 16, ParseExpression(std::get<assembler::Immediate>(operands).value()) });
    } else {
        instruction.data.u32 |= Immediate;
    }

    return instruction;
}

typedef std::tuple<Target> (*JTypeSyntaxFunc)(std::string);

template <uint32_t OpCode, uint32_t Target, JTypeSyntaxFunc Syntax>
object::Instruction JType(const Entry& entry) {
    object::Instruction instruction {};
    instruction.data.u32 = OpCode << 26U;
    instruction.size = 4;

    auto operands = Syntax(entry.operands.value_or(""));
    if constexpr (IsArgSentinel(Target)) {
        instruction.expr_mappings.emplace_back(
            object::ExpressionMapping{ 0, 26, ParseExpression(std::get<assembler::Target>(operands).value()) });
    } else {
        instruction.data.u32 |= Target;
    }

    return instruction;
}

    object::Instruction ParseJALR(const Entry& entry) {
    try {
        // TODO: catch correct exception only
        return RType<0b000000, Arg, 0b00000, Arg, 0b000000, 0b001001, RTypeTuple<RD, RS>>(entry);
    } catch (...) {
        // Try to parse as single register. If it works (it's RS), inject default $31 for RD.
        ParseRegister(entry.operands.value_or(""));
        return RType<0b000000, Arg, 0b00000, Arg, 0b000000, 0b001001, RTypeTuple<RD, RS>>(
            Entry{
                entry.label,
                entry.operation,
                "$31," + entry.operands.value(),
                entry.comment
            }
        );
    }
}

template <uint32_t OpCode>
object::Instruction ParseCOPz(const Entry& entry) {
    // JType looks to be the closest format, so we use it to fill the constant parts
    // of the instruction (OpCode and bit 25).
    auto instruction = JType<OpCode, 0x2000000, JTypeTuple<Target>>(entry);

    if (!entry.operands) {
        throw "missing operation";
    }

    // Add 25 bit Co-processor operation as expression.
    instruction.expr_mappings.emplace_back(object::ExpressionMapping{ 0, 25, ParseExpression(entry.operands.value()) });

    return instruction;
}

object::Instruction ThrowInvalidCoprocessor(const Entry& entry) {
    throw "Instruction not supported by coprocessor: " + entry.operation.value();
}

typedef object::Instruction (*ParseFunc)(const Entry&);
std::unordered_map<std::string, ParseFunc> instructions_fn = {
    { "add",    RType<0b000000, Arg, Arg, Arg, 0b00000, 0b100000, RTypeTuple<RD, RS, RT>> },
    { "addi",   IType<0b001000, Arg, Arg, Arg, ITypeTuple<RT, RS, Immediate>> },
    { "addiu",  IType<0b001001, Arg, Arg, Arg, ITypeTuple<RT, RS, Immediate>> },
    { "addu",   RType<0b000000, Arg, Arg, Arg, 0b00000, 0b100001, RTypeTuple<RD, RS, RT>> },
    { "and",    RType<0b000000, Arg, Arg, Arg, 0b00000, 0b100100, RTypeTuple<RD, RS, RT>> },
    { "andi",   IType<0b001100, Arg, Arg, Arg, ITypeTuple<RT, RS, Immediate>> },
    { "beq",    IType<0b000100, Arg, Arg, Arg, ITypeTuple<RS, RT, Immediate>> },
    { "bgez",   IType<0b000001, Arg, 0b00001, Arg, ITypeTuple<RS, Immediate>> },
    { "bgezal", IType<0b000001, Arg, 0b10001, Arg, ITypeTuple<RS, Immediate>> },
    { "bgtz",   IType<0b000111, Arg, 0b00000, Arg, ITypeTuple<RS, Immediate>> },
    { "blez",   IType<0b000110, Arg, 0b00000, Arg, ITypeTuple<RS, Immediate>> },
    { "bltz",   IType<0b000001, Arg, 0b00000, Arg, ITypeTuple<RS, Immediate>> },
    { "bltzal", IType<0b000001, Arg, 0b10000, Arg, ITypeTuple<RS, Immediate>> },
    { "bne",    IType<0b000101, Arg, Arg, Arg, ITypeTuple<RS, RT, Immediate>> },
    { "break",  RType<0b000000, 0b00000, 0b00000, 0b00000, 0b00000, 0b001101, RTypeNoArgs> },
    { "cfc0",   ThrowInvalidCoprocessor },
    { "cfc1",   RType<0b010001, 0b00010, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "cfc2",   RType<0b010010, 0b00010, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "cfc3",   RType<0b010011, 0b00010, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "cop0",   ParseCOPz<0b010000> },
    { "cop1",   ParseCOPz<0b010001> },
    { "cop2",   ParseCOPz<0b010010> },
    { "cop3",   ParseCOPz<0b010011> },
    { "ctc0",   ThrowInvalidCoprocessor },
    { "ctc1",   RType<0b010001, 0b00110, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "ctc2",   RType<0b010010, 0b00110, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "ctc3",   RType<0b010011, 0b00110, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "div",    RType<0b000000, Arg, Arg, 0b00000, 0b000000, 0b011010, RTypeTuple<RS, RT>> },
    { "divu",   RType<0b000000, Arg, Arg, 0b00000, 0b000000, 0b011011, RTypeTuple<RS, RT>> },
    { "j",      JType<0b000010, Arg, JTypeTuple<Target>> },
    { "jal",    JType<0b000011, Arg, JTypeTuple<Target>> },
    { "jalr",   ParseJALR /* special handling due to optional RD (defaults to $31)*/ },
    { "jr",     RType<0b000000, Arg, 0b00000, 0b00000, 0b000000, 0b001000, RTypeTuple<RS>> },
    { "lb",     IType<0b100000, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lbu",    IType<0b100100, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lh",     IType<0b100001, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lhu",    IType<0b100101, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lui",    IType<0b001111, 0b00000, Arg, Arg, ITypeTuple<RT, Immediate>> },
    { "lw",     IType<0b100011, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lwc0",   ThrowInvalidCoprocessor },
    { "lwc1",   IType<0b110001, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lwc2",   IType<0b110010, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lwc3",   IType<0b110011, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lwl",    IType<0b100010, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "lwr",    IType<0b100110, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "mfc0",   RType<0b010000, 0b00000, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "mfc1",   RType<0b010001, 0b00000, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "mfc2",   RType<0b010010, 0b00000, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "mfc3",   RType<0b010011, 0b00000, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "mfhi",   RType<0b000000, 0b00000, 0b00000, Arg, 0b00000, 0b010000, RTypeTuple<RD>> },
    { "mflo",   RType<0b000000, 0b00000, 0b00000, Arg, 0b00000, 0b010010, RTypeTuple<RD>> },
    { "mtc0",   RType<0b010000, 0b00100, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "mtc1",   RType<0b010001, 0b00100, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "mtc2",   RType<0b010010, 0b00100, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "mtc3",   RType<0b010011, 0b00100, Arg, Arg, 0b00000, 0b000000, RTypeTuple<RT, RD>> },
    { "mthi",   RType<0b000000, Arg, 0b00000, 0b00000, 0b00000, 0b010001, RTypeTuple<RS>> },
    { "mtlo",   RType<0b000000, Arg, 0b00000, 0b00000, 0b00000, 0b010011, RTypeTuple<RS>> },
    { "mult",   RType<0b000000, Arg, Arg, 0b00000, 0b00000, 0b011000, RTypeTuple<RS, RT>> },
    { "multu",  RType<0b000000, Arg, Arg, 0b00000, 0b00000, 0b011001, RTypeTuple<RS, RT>> },
    { "nor",    RType<0b000000, Arg, Arg, Arg, 0b00000, 0b100111, RTypeTuple<RD, RS, RT>> },
    { "or",     RType<0b000000, Arg, Arg, Arg, 0b00000, 0b100101, RTypeTuple<RD, RS, RT>> },
    { "ori",    IType<0b001101, Arg, Arg, Arg, ITypeTuple<RT, RS, Immediate>> },
    { "sb",     IType<0b101000, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "sh",     IType<0b101001, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "sll",    RType<0b000000, 0b00000, Arg, Arg, Arg, 0b000000, RTypeTuple<RD, RT, Shift>> },
    { "sllv",   RType<0b000000, Arg, Arg, Arg, 0b00000, 0b000100, RTypeTuple<RD, RT, RS>> },
    { "slt",    RType<0b000000, Arg, Arg, Arg, 0b00000, 0b101010, RTypeTuple<RD, RS, RT>> },
    { "slti",   IType<0b001010, Arg, Arg, Arg, ITypeTuple<RT, RS, Immediate>> },
    { "sltiu",  IType<0b001011, Arg, Arg, Arg, ITypeTuple<RT, RS, Immediate>> },
    { "sltu",   RType<0b000000, Arg, Arg, Arg, 0b00000, 0b101011, RTypeTuple<RD, RS, RT>> },
    { "sra",    RType<0b000000, 0b00000, Arg, Arg, Arg, 0b000011, RTypeTuple<RD, RT, Shift>> },
    { "srav",   RType<0b000000, Arg, Arg, Arg, 0b00000, 0b000111, RTypeTuple<RD, RT, RS>> },
    { "srl",    RType<0b000000, 0b00000, Arg, Arg, Arg, 0b000010, RTypeTuple<RD, RT, Shift>> },
    { "srlv",   RType<0b000000, Arg, Arg, Arg, 0b00000, 0b000110, RTypeTuple<RD, RT, RS>> },
    { "sub",    RType<0b000000, Arg, Arg, Arg, 0b00000, 0b100010, RTypeTuple<RD, RS, RT>> },
    { "subu",   RType<0b000000, Arg, Arg, Arg, 0b00000, 0b100011, RTypeTuple<RD, RS, RT>> },
    { "sw",     IType<0b101011, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "swc0",   ThrowInvalidCoprocessor },
    { "swc1",   IType<0b111001, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "swc2",   IType<0b111010, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "swc3",   IType<0b111011, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "swl",    IType<0b101010, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "swr",    IType<0b101110, Arg, Arg, Arg, ITypeOffset<RT, Immediate, RS>> },
    { "syscall",RType<0b000000, 0b00000, 0b00000, 0b00000, 0b00000, 0b001100, RTypeNoArgs> },
    { "xor",    RType<0b000000, Arg, Arg, Arg, 0b00000, 0b100110, RTypeTuple<RD, RS, RT>> },
    { "xori",   IType<0b001110, Arg, Arg, Arg, ITypeTuple<RT, RS, Immediate>> },

    // CP-0 Instructions
    // Note: these can also be invoked using copz generic instruction.
    { "rfe",    RType<0b010000, 0b10000, 0b00000, 0b00000, 0b00000, 0b010000, RTypeNoArgs> },
    { "tlbp",   RType<0b010000, 0b10000, 0b00000, 0b00000, 0b00000, 0b001000, RTypeNoArgs> },
    { "tlbr",   RType<0b010000, 0b10000, 0b00000, 0b00000, 0b00000, 0b000001, RTypeNoArgs> },
    { "tlbwi",  RType<0b010000, 0b10000, 0b00000, 0b00000, 0b00000, 0b000010, RTypeNoArgs> },
    { "tlbwr",  RType<0b010000, 0b10000, 0b00000, 0b00000, 0b00000, 0b000110, RTypeNoArgs> }
};
}

MipsAssemblerTarget::MipsAssemblerTarget(support::Endian endianness) : endianness(endianness) { };
MipsAssemblerTarget::~MipsAssemblerTarget() = default;

support::Endian MipsAssemblerTarget::GetEndianness() {
    return endianness;
}

object::Instruction MipsAssemblerTarget::EmitInstruction(const Entry& entry) {
    // TODO: return proper error for missing instruction.
    auto instruction = instructions_fn[entry.operation.value()](entry);

    if (endianness != support::HostEndian) {
        support::EndianSwap(&instruction.data.u32);
    }

    return instruction;
}

}