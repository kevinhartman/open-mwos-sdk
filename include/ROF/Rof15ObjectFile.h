#pragma once

#include "ObjectFile.h"
#include "Rof15Header.h"
#include "SerializableStruct.h"

#include <memory>
#include <variant>
#include <vector>

namespace rof {

using SerializableExternDefinition = SerializableStruct<SerializableString, uint16_t, uint32_t>;

enum ExternDefinitionType : uint16_t {
    Data_NonRemote_Uninitialized = 0b000,
    Data_NonRemote_Initialized = 0b001,
    Data_Remote_Uninitialized = 0b010,
    Data_Remote_Initialized = 0b011,

    Code_NotCommonBlock_CodeLabel = 0b100,
    Code_NotCommonBlock_SetLabel = 0b101,
    Code_NotCommonBlock_EquLabel = 0b110,

    Code_CommonBlock_CodeLabel = 0b100000100,
    Code_CommonBlock_SetLabel = 0b100000101,
    Code_CommonBlock_EquLabel = 0b100000110
};

struct ExternDefinition : SerializableExternDefinition {
    auto& Name() { return std::get<0>(*this); }
    auto& Type() { return std::get<1>(*this); }
    auto& SymbolValue() { return std::get<2>(*this); }
};

using SerializableReference = SerializableStruct<uint8_t, uint8_t, uint16_t, uint32_t, uint32_t>;
struct Reference : SerializableReference {
    auto& BitNumber() { return std::get<0>(*this); }
    auto& FieldLength() { return std::get<1>(*this); }
    auto& LocationFlag() { return std::get<2>(*this); }
    auto& LocalOffset() { return std::get<3>(*this); }
    auto& ExprTreeIndex() { return std::get<4>(*this); }
};

using ExpressionVal = uint32_t;
using SerializableExprRef = SerializableStruct<uint16_t, uint32_t>;
struct ExpressionRef : SerializableExprRef {
    auto& Flags() { return std::get<0>(*this); }
    auto& Value() { return std::get<1>(*this); }
};

enum class ExpressionOperator : uint16_t {
    NumericConstant,
    Reference,
    Hi,
    Lo,
    ArithmeticNegation,
    BitwiseNegation,
    BitwiseAnd,
    BitwiseOr,
    BitwiseXor,
    Multiplication,
    Division,
    Addition,
    Subtraction,
    LeftShift,
    RightShift,
    ArithmeticRightShift,
    High
};

struct ExpressionTree;
using ExpressionTreeOperand = std::variant<ExpressionVal, ExpressionRef, std::unique_ptr<ExpressionTree>>;
struct ExpressionTree {
    explicit ExpressionTree(ExpressionOperator op) : op(op) {}
    ExpressionOperator op;
    ExpressionTreeOperand operand1 = std::unique_ptr<ExpressionTree>(nullptr);
    ExpressionTreeOperand operand2 = std::unique_ptr<ExpressionTree>(nullptr);
};

}