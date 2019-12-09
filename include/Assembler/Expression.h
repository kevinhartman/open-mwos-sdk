#pragma once

#include "Visitor.h"

#include <memory>
#include <string>

namespace assembler {

using ExpressionVisitorTypes = visitor::VisitorTypes<
    const struct NumericConstantExpression&,
    const struct ReferenceExpression&,
    const struct HiExpression&,
    const struct HighExpression&,
    const struct LoExpression&,
    const struct NegationExpression&,
    const struct BitwiseNotExpression&,
    const struct BitwiseAndExpression&,
    const struct BitwiseOrExpression&,
    const struct BitwiseXorExpression&,
    const struct MultiplicationExpression&,
    const struct DivisionExpression&,
    const struct AdditionExpression&,
    const struct SubtractionExpression&,
    const struct LogicalLeftShiftExpression&,
    const struct LogicalRightShiftExpression&,
    const struct ArithmeticRightShiftExpression&
>;

using Expression = ExpressionVisitorTypes::Visitable;
using ExpressionVisitor = ExpressionVisitorTypes::Visitor;

template<typename T>
class ValueExpression {
public:
    virtual const T& Value() const = 0;
};

template <typename T, typename TValue>
struct ValueExpressionBase : public ValueExpression<TValue>, public ExpressionVisitorTypes::VisitableImpl<T> {
    ValueExpressionBase(TValue value): value(value) {}
    ~ValueExpressionBase() override = default;

    const TValue& Value() const override {
        return value;
    }

    TValue value {};
};

class PrefixExpression {
public:
    virtual const Expression& Left() const = 0;
};

template <typename T>
struct PrefixExpressionBase : public PrefixExpression, public ExpressionVisitorTypes::VisitableImpl<T> {
    PrefixExpressionBase(std::unique_ptr<Expression> left): left(std::move(left)) {}
    ~PrefixExpressionBase() override = default;

    const Expression& Left() const override {
        return *left;
    }

    std::unique_ptr<Expression> left {};
};

class InfixExpression {
public:
    virtual const Expression& Left() const = 0;
    virtual const Expression& Right() const = 0;
};

template <typename T>
struct InfixExpressionBase : public InfixExpression, public ExpressionVisitorTypes::VisitableImpl<T> {
    InfixExpressionBase(std::unique_ptr<Expression> left, std::unique_ptr<Expression> right)
        : left(std::move(left)), right(std::move(right)) {}
    ~InfixExpressionBase() override = default;

    const Expression& Left() const override {
        return *left;
    }

    const Expression& Right() const override {
        return *right;
    }

    std::unique_ptr<Expression> left {};
    std::unique_ptr<Expression> right {};
};

struct NumericConstantExpression : public ValueExpressionBase<NumericConstantExpression, uint32_t> { using ValueExpressionBase::ValueExpressionBase; };
struct ReferenceExpression : public ValueExpressionBase<ReferenceExpression, std::string> { using ValueExpressionBase::ValueExpressionBase; };
struct NegationExpression : public PrefixExpressionBase<NegationExpression> { using PrefixExpressionBase::PrefixExpressionBase; };
struct BitwiseNotExpression : public PrefixExpressionBase<BitwiseNotExpression> { using PrefixExpressionBase::PrefixExpressionBase; };
struct BitwiseAndExpression : public InfixExpressionBase<BitwiseAndExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct BitwiseOrExpression : public InfixExpressionBase<BitwiseOrExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct BitwiseXorExpression : public InfixExpressionBase<BitwiseXorExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct MultiplicationExpression : public InfixExpressionBase<MultiplicationExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct DivisionExpression : public InfixExpressionBase<DivisionExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct AdditionExpression : public InfixExpressionBase<AdditionExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct SubtractionExpression : public InfixExpressionBase<SubtractionExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct LogicalLeftShiftExpression : public InfixExpressionBase<LogicalLeftShiftExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct LogicalRightShiftExpression : public InfixExpressionBase<LogicalRightShiftExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct ArithmeticRightShiftExpression : public InfixExpressionBase<ArithmeticRightShiftExpression> { using InfixExpressionBase::InfixExpressionBase; };

struct HiExpression : public InfixExpressionBase<HiExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct HighExpression : public InfixExpressionBase<HighExpression> { using InfixExpressionBase::InfixExpressionBase; };
struct LoExpression : public InfixExpressionBase<LoExpression> { using InfixExpressionBase::InfixExpressionBase; };

}