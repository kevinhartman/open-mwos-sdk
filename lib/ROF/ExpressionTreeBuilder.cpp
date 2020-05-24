#include <ExpressionTreeBuilder.h>
#include <Rof15ObjectFile.h>

namespace rof {
    using namespace expression;

    void ExpressionTreeBuilder::Visit(const NumericConstantExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::NumericConstant);
        result->operand1 = expr.value;
    }

    // TODO: implement
    void ExpressionTreeBuilder::Visit(const ReferenceExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::Reference);

        ExpressionRef reference {};
        reference.Flags() = 0;
        reference.Value() = 0;

        result->operand1 = std::move(reference);

        throw std::runtime_error("unimplemented");
    }

    void ExpressionTreeBuilder::Visit(const HiExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::Hi);
        result->operand1 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const HighExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::High);
        result->operand1 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const LoExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::Lo);
        result->operand1 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const NegationExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::ArithmeticNegation);
        result->operand1 = SubTree(expr.Left());
    }

    void ExpressionTreeBuilder::Visit(const BitwiseNotExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::BitwiseNegation);
        result->operand1 = SubTree(expr.Left());
    }

    void ExpressionTreeBuilder::Visit(const BitwiseAndExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::BitwiseAnd);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const BitwiseOrExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::BitwiseOr);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());    }

    void ExpressionTreeBuilder::Visit(const BitwiseXorExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::BitwiseXor);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const MultiplicationExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::Multiplication);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const DivisionExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::Division);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const AdditionExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::Addition);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const SubtractionExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::Subtraction);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const LogicalLeftShiftExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::LeftShift);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const LogicalRightShiftExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::RightShift);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    void ExpressionTreeBuilder::Visit(const ArithmeticRightShiftExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::ArithmeticRightShift);
        result->operand1 = SubTree(expr.Left());
        result->operand2 = SubTree(expr.Right());
    }

    std::unique_ptr<ExpressionTree> ExpressionTreeBuilder::SubTree(const expression::Expression& expr) const {
        ExpressionTreeBuilder builder(object_file);
        return builder.Build(expr);
    }

    ExpressionTreeBuilder::ExpressionTreeBuilder(const object::ObjectFile& object_file) :
        object_file(object_file) {}

    std::unique_ptr<ExpressionTree> ExpressionTreeBuilder::Build(const Expression& expression) {
        expression.Accept(*this);
        return std::move(result);
    }
}