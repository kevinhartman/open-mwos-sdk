#pragma once

#include <Expression.h>

#include <memory>
#include <vector>

namespace object {
class ObjectFile;
}

namespace rof {
class ExpressionTree;
class ExpressionTreeBuilder : public expression::ExpressionVisitor {
public:
    explicit ExpressionTreeBuilder(const object::ObjectFile&, std::vector<std::string>&);
    std::unique_ptr<ExpressionTree> Build(const expression::Expression& expression);

protected:
    void Visit(const expression::NumericConstantExpression&) override;
    void Visit(const expression::ReferenceExpression&) override;
    void Visit(const expression::HiExpression&) override;
    void Visit(const expression::HighExpression&) override;
    void Visit(const expression::LoExpression&) override;
    void Visit(const expression::NegationExpression&) override;
    void Visit(const expression::BitwiseNotExpression&) override;
    void Visit(const expression::BitwiseAndExpression&) override;
    void Visit(const expression::BitwiseOrExpression&) override;
    void Visit(const expression::BitwiseXorExpression&) override;
    void Visit(const expression::MultiplicationExpression&) override;
    void Visit(const expression::DivisionExpression&) override;
    void Visit(const expression::AdditionExpression&) override;
    void Visit(const expression::SubtractionExpression&) override;
    void Visit(const expression::LogicalLeftShiftExpression&) override;
    void Visit(const expression::LogicalRightShiftExpression&) override;
    void Visit(const expression::ArithmeticRightShiftExpression&) override;

    std::unique_ptr<ExpressionTree> result {};
private:
    std::unique_ptr<ExpressionTree> SubTree(const expression::Expression& expr) const;

    const object::ObjectFile& object_file;
    std::vector<std::string>& extern_refs;
};
}