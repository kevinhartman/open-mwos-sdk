#pragma once

#include "ObjectFile.h"
#include "Rof15Header.h"
#include "SerializableStruct.h"

#include <cassert>
#include <memory>
#include <variant>
#include <vector>

namespace rof {
enum class DefinitionType : uint16_t {
    Data = 0,
    Code = 1 << 2,
    Uninitialized = 0,
    Initialized = 1,
    UninitializedRemote = 2,
    InitializedRemote = 3,
    Set = 1,
    Equ = 2,
    CommonBlock = 1 << 8
};

inline DefinitionType operator|(DefinitionType a, DefinitionType b)
{
    return static_cast<DefinitionType>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

inline DefinitionType GetDefinitionType(object::SymbolInfo::Type type) {
    switch (type) {
        case object::SymbolInfo::Code:
            return DefinitionType::Code;
        case object::SymbolInfo::Equ:
            return DefinitionType::Code | DefinitionType::Equ;
        case object::SymbolInfo::Set:
            return DefinitionType::Code | DefinitionType::Set;
        case object::SymbolInfo::InitData:
            return DefinitionType::Initialized;
        case object::SymbolInfo::UninitData:
            return DefinitionType::Uninitialized;
        case object::SymbolInfo::RemoteInitData:
            return DefinitionType::InitializedRemote;
        case object::SymbolInfo::RemoteUninitData:
            return DefinitionType::UninitializedRemote;
        default:
            assert("unknown definition type");
            return DefinitionType::Data;
    }
}

using SerializableExternDefinition = SerializableStruct<SerializableString, uint16_t, uint32_t>;
struct ExternDefinition : SerializableExternDefinition {
    auto& Name() { return std::get<0>(*this); }
    auto& Type() { return std::get<1>(*this); }
    auto& SymbolValue() { return std::get<2>(*this); }
};

enum class ReferenceFlags : uint16_t {
    Data = 0,
    Code = 1 << 5,
    Remote = 1 << 9,
    Signed = 1 << 10
};

inline ReferenceFlags operator|(ReferenceFlags a, ReferenceFlags b)
{
    return static_cast<ReferenceFlags>(static_cast<uint16_t>(a) | static_cast<uint16_t>(b));
}

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