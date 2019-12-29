#include "PrinterHelpers.h"

#include "AssemblerTarget.h"
#include "Expression.h"

#include <ostream>
#include <sstream>
#include <string>

namespace assembler {
struct PrintExpressionVisitor : ExpressionVisitor {
    void Visit(const NumericConstantExpression& expr) override {
        result << expr.value;
    }

    void Visit(const ReferenceExpression& expr) override {
        result << expr.value;
    }

    void Visit(const HiExpression& expr) override {
        VisitInfix(*expr.left, "(", *expr.right);
        result << ")";
    }

    void Visit(const HighExpression& expr) override {
        VisitInfix(*expr.left, "(", *expr.right);
        result << ")";
    }

    void Visit(const LoExpression& expr) override {
        VisitInfix(*expr.left, "(", *expr.right);
        result << ")";
    }

    void VisitPrefix(std::string op, const Expression& left) {
        PrintExpressionVisitor visitor {};
        left.Accept(visitor);

        result << "(" << op << visitor.result.str() << ")";
    }

    void Visit(const NegationExpression& expr) override {
        VisitPrefix("-", *expr.left);
    }

    void Visit(const BitwiseNotExpression& expr) override {
        VisitPrefix("~", *expr.left);
    }

    void VisitInfix(const Expression& left, std::string op, const Expression& right) {
        PrintExpressionVisitor v_left {};
        left.Accept(v_left);

        PrintExpressionVisitor v_right {};
        right.Accept(v_right);

        result << "(" << v_left.result.str() << op << v_right.result.str() << ")";
    }

    void Visit(const BitwiseAndExpression& expr) override {
        VisitInfix(*expr.left, "&", *expr.right);
    }

    void Visit(const BitwiseOrExpression& expr) override {
        VisitInfix(*expr.left, "|", *expr.right);
    }

    void Visit(const BitwiseXorExpression& expr) override {
        VisitInfix(*expr.left, "^", *expr.right);
    }

    void Visit(const MultiplicationExpression& expr) override {
        VisitInfix(*expr.left, "*", *expr.right);
    }

    void Visit(const DivisionExpression& expr) override {
        VisitInfix(*expr.left, "/", *expr.right);
    }

    void Visit(const AdditionExpression& expr) override {
        VisitInfix(*expr.left, "+", *expr.right);
    }

    void Visit(const SubtractionExpression& expr) override {
        VisitInfix(*expr.left, "-", *expr.right);
    }

    void Visit(const LogicalLeftShiftExpression& expr) override {
        VisitInfix(*expr.left, "<<", *expr.right);
    }

    void Visit(const LogicalRightShiftExpression& expr) override {
        VisitInfix(*expr.left, ">>", *expr.right);
    }

    void Visit(const ArithmeticRightShiftExpression& expr) override {
        throw "unimplemented";
    }

    std::stringstream result {};
};

std::ostream& operator<<(std::ostream& os, const Expression& expr) {
    PrintExpressionVisitor print_visitor {};
    expr.Accept(print_visitor);

    os << print_visitor.result.str();
    return os;
}

std::ostream &operator<<(std::ostream &os, const ExpressionMapping &expr) {
    os << "{ offset: " << expr.offset << " bit_count: " << expr.bit_count << " expression: " << *expr.expression;
    return os;
}

}