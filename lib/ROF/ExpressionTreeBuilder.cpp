#include <ExpressionTreeBuilder.h>
#include <Rof15ObjectFile.h>

namespace rof {
    using namespace expression;

    void ExpressionTreeBuilder::Visit(const NumericConstantExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::NumericConstant);
        result->operand1 = expr.value;
    }

    void ExpressionTreeBuilder::Visit(const ReferenceExpression& expr) {
        result = std::make_unique<ExpressionTree>(ExpressionOperator::Reference);

        ExpressionRef reference {};
        reference.Flags() = 0;
        reference.Value() = 0;

        auto sym_itr = object_file.psect.symbols.find(expr.Value());

        if (sym_itr != object_file.psect.symbols.end()) {
            // local reference
            auto& symbol = sym_itr->second;
            reference.Flags() = static_cast<uint16_t>(GetDefinitionType(symbol.type));
            reference.Flags() |= 0b1000000000000U; // local

            // TODO: handle "common blocks" (from com directive) here (bit 8)

            // TODO: handle nullopt, or make sym val non-optional if NA
            reference.Value() = symbol.value.value();
        } else {
            // external reference
            auto extern_ref = std::find(extern_refs.begin(), extern_refs.end(), expr.Value());
            if (extern_ref != extern_refs.end()) {
                // reuse existing extern reference
                reference.Value() = std::distance(extern_refs.begin(), extern_ref);
            } else {
                // new extern ref
                reference.Value() = extern_refs.size();
                extern_refs.emplace_back(expr.Value());
            }
            // TODO: implement alignment requirement support (bits 3-4) and relative ref (bit 7).
        }

        result->operand1 = std::move(reference);
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
        ExpressionTreeBuilder builder(object_file, extern_refs);
        return builder.Build(expr);
    }

    ExpressionTreeBuilder::ExpressionTreeBuilder(const object::ObjectFile& object_file, std::vector<std::string>& extern_refs) :
        object_file(object_file), extern_refs(extern_refs) {}

    std::unique_ptr<ExpressionTree> ExpressionTreeBuilder::Build(const Expression& expression) {
        expression.Accept(*this);
        return std::move(result);
    }
}