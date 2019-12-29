#include <catch2/catch.hpp>

#include "ComparisonHelpers.h"
#include "PrinterHelpers.h"

#include "Expression.h"
#include "ExpressionLexer.h"
#include "ExpressionParser.h"

#include <string>
#include <typeinfo>

namespace assembler {

struct StringToExpression {
    std::string input_expression;
    std::shared_ptr<Expression> expected_expression;
};

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