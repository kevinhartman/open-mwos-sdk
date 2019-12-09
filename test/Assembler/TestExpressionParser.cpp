#include <catch2/catch.hpp>

#include "Expression.h"
#include "ExpressionLexer.h"
#include "ExpressionParser.h"

#include <string>
#include <sstream>
#include <typeinfo>

namespace assembler {

struct StringToExpression {
    std::string input_expression;
    std::shared_ptr<Expression> expected_expression;
};

bool operator==(const Expression& e1, const Expression& e2) {
    if (typeid(e1) != typeid(e2)) {
        return false;
    }

    // Handle prefix case.
    const auto* e1_prefix = dynamic_cast<const PrefixExpression*>(&e1);
    if (e1_prefix != nullptr) {
        return e1_prefix->Left() == dynamic_cast<const PrefixExpression*>(&e2)->Left();
    }

    // Handle infix case.
    const auto* e1_infix = dynamic_cast<const InfixExpression*>(&e1);
    if (e1_infix != nullptr) {
        return e1_infix->Left() == dynamic_cast<const InfixExpression*>(&e2)->Left()
            && e1_infix->Right() == dynamic_cast<const InfixExpression*>(&e2)->Right();
    }

    // Handle numeric constant.
    const auto* e1_number = dynamic_cast<const ValueExpression<uint32_t>*>(&e1);
    if (e1_number != nullptr) {
        return e1_number->Value() == dynamic_cast<const ValueExpression<uint32_t>*>(&e2)->Value();
    }

    // Handle string constant.
    const auto* e1_string = dynamic_cast<const ValueExpression<std::string>*>(&e1);
    if (e1_string != nullptr) {
        return e1_string->Value() == dynamic_cast<const ValueExpression<std::string>*>(&e2)->Value();
    }

    assert(false); // unhandled expression type!
    return false;
}


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

        result << op << visitor.result.str();
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

        result << v_left.result.str() << op << v_right.result.str();
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

SCENARIO("Valid expressions are properly parsed", "[parser]") {
    GIVEN("each expression string") {
        auto pair = GENERATE(values<StringToExpression>({
            {
                "0",
                std::make_shared<NumericConstantExpression>(0)
            },
            {
                // Test multiplication precedence.
                "5+10*2",
                std::make_shared<AdditionExpression>(
                    std::make_unique<NumericConstantExpression>(5),
                    std::make_unique<MultiplicationExpression>(
                        std::make_unique<NumericConstantExpression>(10),
                        std::make_unique<NumericConstantExpression>(2)
                    )
                )
            },
            {
                // Test multiplication precedence despite natural right-associativity.
                "5*10+2",
                std::make_shared<AdditionExpression>(
                    std::make_unique<MultiplicationExpression>(
                        std::make_unique<NumericConstantExpression>(5),
                        std::make_unique<NumericConstantExpression>(10)
                    ),
                    std::make_unique<NumericConstantExpression>(2)
                )
            }
        }));

        WHEN("the expression is parsed") {
            ExpressionLexer lexer(pair.input_expression);
            ExpressionParser parser(lexer);

            std::unique_ptr<Expression> expression = parser.Parse();

            THEN("the expression tree is correct") {
                REQUIRE(*expression == *pair.expected_expression);
            }
        }
    }
}

}