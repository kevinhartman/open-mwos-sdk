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

SCENARIO("Valid expressions are properly parsed", "[expression]") {
    GIVEN("each expression string") {
        auto pair = GENERATE(values<StringToExpression>({
            {
                "0",
                std::make_shared<NumericConstantExpression>(0)
            },
            {
                // Test max value decimal.
                "4294967295",
                std::make_shared<NumericConstantExpression>(4294967295)
            },
            {
                // Test max value hex.
                "0xFFFFFFFF",
                std::make_shared<NumericConstantExpression>(0xFFFFFFFF)
            },
            {
                // Test max value hex.
                "$FFFFFFFF",
                std::make_shared<NumericConstantExpression>(0xFFFFFFFF)
            },
            {
                // Test max value bin.
                "%11111111111111111111111111111111",
                std::make_shared<NumericConstantExpression>(0b11111111111111111111111111111111)
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
            },
            {
                // Test subexpression / grouping.
                "5*(10+2)",
                std::make_shared<MultiplicationExpression>(
                    std::make_unique<NumericConstantExpression>(5),
                    std::make_unique<AdditionExpression>(
                        std::make_unique<NumericConstantExpression>(10),
                        std::make_unique<NumericConstantExpression>(2)
                    )
                )
            },
            {
                // Test negation prefix expression.
                "-5",
                std::make_shared<NegationExpression>(
                    std::make_unique<NumericConstantExpression>(5)
                )
            },
            {
                // Test double-negation prefix expression.
                "--5",
                std::make_shared<NegationExpression>(
                    std::make_unique<NegationExpression>(
                        std::make_unique<NumericConstantExpression>(5)
                    )
                )
            },
            {
                // Test subtraction of negated expression.
                "0xAFF--2",
                std::make_shared<SubtractionExpression>(
                    std::make_unique<NumericConstantExpression>(0xAFF),
                    std::make_unique<NegationExpression>(
                        std::make_unique<NumericConstantExpression>(2)
                    )
                )
            },
            {
                // Test prefix precedence.
                "-5*10",
                std::make_shared<MultiplicationExpression>(
                    std::make_unique<NegationExpression>(
                        std::make_unique<NumericConstantExpression>(5)
                    ),
                    std::make_unique<NumericConstantExpression>(10)
                )
            },
            {
                // Test nested groups don't impact func-like operators.
                // Note: this might not work in real OS9 asm expressions, but it works in C!
                "((((high))))(((((label)))))",
                std::make_shared<HighExpression>(
                    std::make_unique<ReferenceExpression>("high"),
                    std::make_unique<ReferenceExpression>("label")
                )
            },
            {
                // Test precedence overrides despite natural right-associativity.
                "-hi(123)|lo(123)^^high(1)*3/%011+label1-label2<<3>>3",
                std::make_shared<LogicalRightShiftExpression>(
                    std::make_unique<LogicalLeftShiftExpression>(
                        std::make_unique<SubtractionExpression>(
                            std::make_unique<AdditionExpression>(
                                std::make_unique<DivisionExpression>(
                                    std::make_unique<MultiplicationExpression>(
                                        std::make_unique<BitwiseXorExpression>(
                                            std::make_unique<BitwiseOrExpression>(
                                                std::make_unique<NegationExpression>(
                                                    std::make_unique<HiExpression>(
                                                        std::make_unique<ReferenceExpression>("hi"),
                                                        std::make_unique<NumericConstantExpression>(123)
                                                    )
                                                ),
                                                std::make_unique<LoExpression>(
                                                    std::make_unique<ReferenceExpression>("lo"),
                                                    std::make_unique<NumericConstantExpression>(123)
                                                )
                                            ),
                                            std::make_unique<BitwiseNotExpression>(
                                                std::make_unique<HighExpression>(
                                                    std::make_unique<ReferenceExpression>("high"),
                                                    std::make_unique<NumericConstantExpression>(1)
                                                )
                                            )
                                        ),
                                        std::make_unique<NumericConstantExpression>(3)
                                    ),
                                    std::make_unique<NumericConstantExpression>(0b011)
                                ),
                                std::make_unique<ReferenceExpression>("label1")
                            ),
                            std::make_unique<ReferenceExpression>("label2")
                        ),
                        std::make_unique<NumericConstantExpression>(3)
                    ),
                    std::make_unique<NumericConstantExpression>(3)
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

SCENARIO("Unsatisfied expression contexts throw", "[expression]") {
    GIVEN("each expression string") {
        auto input_expression = GENERATE(values<std::string>({
            // Test empty expressions
            "",
            "()",
            "5+hello*()",
            "",
            "hi()",
            "-lo()",
            "6+high()",

            // Test invalid prefix
            "+",
            "(+)",
            "<<4",

            // Test prefix followed by missing expression
            "(-)",
            "(^)",
            "(~)",

            // Test infix with invalid right expression
            "5++2",
            "4*",
        }));

        WHEN("the expression is parsed") {
            ExpressionLexer lexer(input_expression);
            ExpressionParser parser(lexer);

            THEN("ExpectedExpressionException is thrown") {
                REQUIRE_THROWS_AS(parser.Parse(), ExpectedExpressionException);
            }
        }
    }
}

SCENARIO("Unsatisfied token contexts throw", "[expression]") {
    GIVEN("each expression string") {
        auto input_expression = GENERATE(values<std::string>({
            "4hello",
            "(5+(45)3)"
        }));

        WHEN("the expression is parsed") {
            ExpressionLexer lexer(input_expression);
            ExpressionParser parser(lexer);

            THEN("ExpectedTokenException is thrown") {
                REQUIRE_THROWS_AS(parser.Parse(), ExpectedTokenException);
            }
        }
    }
}

SCENARIO("Expressions with invalid func-like operators throw", "[expression]") {
    GIVEN("each expression string") {
        auto input_expression = GENERATE(values<std::string>({
            "hello(34)",
            "(5+(45))(label)",
            "5(5)"
        }));

        WHEN("the expression is parsed") {
            ExpressionLexer lexer(input_expression);
            ExpressionParser parser(lexer);

            THEN("InvalidFuncLikeOperatorException is thrown") {
                REQUIRE_THROWS_AS(parser.Parse(), InvalidFuncLikeOperatorException);
            }
        }
    }
}

SCENARIO("Expressions out of range numeric values throw", "[expression]") {
    GIVEN("each expression string") {
        auto input_expression = GENERATE(values<std::string>({
            // Only possible to test this with decimal, since the Lexer cannot produce out of range
            // numeric tokens for any other n-ary.
            "4294967296",
            "004294967296",
            "900000000000"
        }));

        WHEN("the expression is parsed") {
            ExpressionLexer lexer(input_expression);
            ExpressionParser parser(lexer);

            THEN("NumericExpressionOutOfRangeException is thrown") {
                REQUIRE_THROWS_AS(parser.Parse(), NumericExpressionOutOfRangeException);
            }
        }
    }
}

}