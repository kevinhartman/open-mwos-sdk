#include <catch2/catch.hpp>

#include "ExpressionLexer.h"

#include <string>
#include <tuple>
#include <vector>

namespace assembler {

struct ExpressionToTokens {
    std::string input_expression;
    std::vector<Token> expected_tokens;
};

struct ExpressionToFailureIndex {
    std::string input_expression;
    size_t expected_token_failure_index;
};

const std::vector<Token> IgnoreTokens {};

bool operator==(const Token& t1, const Token& t2) {
    return std::tie(t1.type, t1.text) == std::tie(t2.type, t2.text);
}

std::ostream& operator<<(std::ostream& os, const Token& token) {
    os << std::endl << "  { " << "type: " << token.type << " text: " << token.text << " }";
    return os;
}

SCENARIO("Valid expressions are properly tokenized", "[lexer]") {

    GIVEN("each input line") {
        auto pair = GENERATE(values<ExpressionToTokens>({
            {
                "0",
                {
                    { TokenType::DecimalConstant, "0" }
                }
            },
            {
                std::string(12, '0'),
                {
                    { TokenType::DecimalConstant, std::string(12, '0') }
                }
            },
            {
                "0x0",
                {
                    { TokenType::HexConstant, "0x0" }
                }
            },
            {
                "0x" + std::string(8, 'F'),
                {
                    { TokenType::HexConstant, "0x" + std::string(8, 'F') }
                }
            },
            {
                "$0",
                {
                    { TokenType::HexConstant, "$0" }
                }
            },
            {
                "$" + std::string(8, 'F'),
                {
                    { TokenType::HexConstant, "$" + std::string(8, 'F') }
                }
            },
            {
                "%0",
                {
                    { TokenType::BinaryConstant, "%0" }
                }
            },
            {
                "%" + std::string(32, '1'),
                {
                    { TokenType::BinaryConstant, "%" + std::string(32, '1') }
                }
            },
            {
                "'0'",
                {
                    { TokenType::CharConstant, "'0'" }
                }
            },
            {
                "_0",
                {
                    { TokenType::SymbolicName, "_0" }
                }
            },
            {
                ".",
                {
                    { TokenType::Period, "." }
                }
            },
            {
                "-",
                {
                    { TokenType::Minus, "-" }
                }
            },
            {
                "^",
                {
                    { TokenType::Hat, "^" }
                }
            },
            {
                "~",
                {
                    { TokenType::Tilde, "~" }
                }
            },
            {
                "&",
                {
                    { TokenType::Ampersand, "&" }
                }
            },
            {
                "!",
                {
                    { TokenType::Bang, "!" }
                }
            },
            {
                "|",
                {
                    { TokenType::Pipe, "|" }
                }
            },
            {
                "*",
                {
                    { TokenType::Asterisk,         "*" }
                }
            },
            {
                "/",
                {
                    { TokenType::ForwardSlash,      "/" }
                }
            },
            {
                "+",
                {
                    { TokenType::Plus,              "+" }
                }
            },
            {
                "<<",
                {
                    { TokenType::DoubleLeftCarrot,  "<<" }
                }
            },
            {
                ">>",
                {
                    { TokenType::DoubleRightCarrot, ">>" }
                }
            },
            {
                "(",
                {
                    { TokenType::LeftParen,         "(" }
                }
            },
            {
                ")",
                {
                    { TokenType::RightParen,        ")" }
                }
            },
            {
                "hi(5)",
                {
                    { TokenType::SymbolicName,     "hi" },
                    { TokenType::LeftParen,        "(" },
                    { TokenType::DecimalConstant,  "5" },
                    { TokenType::RightParen,       ")" },
                }
            },
            {
                "(hi(5))",
                {
                    { TokenType::LeftParen,        "(" },
                    { TokenType::SymbolicName,     "hi" },
                    { TokenType::LeftParen,        "(" },
                    { TokenType::DecimalConstant,  "5" },
                    { TokenType::RightParen,       ")" },
                    { TokenType::RightParen,       ")" }
                }
            },
            {
                "5+0xDEADBEEF",
                {
                    { TokenType::DecimalConstant,   "5" },
                    { TokenType::Plus, "+" },
                    { TokenType::HexConstant, "0xDEADBEEF" }
                }
            },
            {
                "$1A*(%00110>>0x4+_hello)",
                {
                    { TokenType::HexConstant, "$1A" },
                    { TokenType::Asterisk, "*" },
                    { TokenType::LeftParen, "(" },
                    { TokenType::BinaryConstant, "%00110" },
                    { TokenType::DoubleRightCarrot, ">>" },
                    { TokenType::HexConstant, "0x4" },
                    { TokenType::Plus, "+" },
                    { TokenType::SymbolicName, "_hello" },
                    { TokenType::RightParen, ")" }
                }
            },
            {
                "0000-~' '/(5+-%0)^@-_&0xF-*",
                {
                    { TokenType::DecimalConstant, "0000" },
                    { TokenType::Minus, "-" },
                    { TokenType::Tilde, "~" },
                    { TokenType::CharConstant, "' '"},
                    { TokenType::ForwardSlash, "/" },
                    { TokenType::LeftParen, "(" },
                    { TokenType::DecimalConstant, "5" },
                    { TokenType::Plus, "+" },
                    { TokenType::Minus, "-" },
                    { TokenType::BinaryConstant, "%0" },
                    { TokenType::RightParen, ")" },
                    { TokenType::Hat, "^" },
                    { TokenType::SymbolicName, "@" },
                    { TokenType::Minus, "-" },
                    { TokenType::SymbolicName, "_" },
                    { TokenType::Ampersand, "&" },
                    { TokenType::HexConstant, "0xF" },
                    { TokenType::Minus, "-" },
                    { TokenType::Asterisk, "*" }
                }
            }
        }));

        WHEN("the expression is lexed") {
            std::vector<Token> tokens {};

            ExpressionLexer lexer(pair.input_expression);
            while (lexer.HasNext()) {
                Token token;
                lexer = lexer.Next(token);

                tokens.emplace_back(token);
            }

            THEN("the tokens are as expected") {
                REQUIRE(tokens == pair.expected_tokens);
            }
        }
    }
}

SCENARIO("Malformed expressions cause the proper lexer failure", "[lexer]") {
    GIVEN("expressions that contain an unexpected character sequence") {
        auto pair = GENERATE(values<ExpressionToFailureIndex>({
            { "`", 0 },
            { " 8+2", 0 },
            { "label+ 5", 2 },
            { "label+5>4", 3 },
            { "10<4", 1 },
            { " ", 0}
        }));

        WHEN("the expression is lexed") {
            THEN("UnhandledTokenException is thrown when parsing the candidate token expected to fail") {
                ExpressionLexer lexer(pair.input_expression);

                size_t index = 0;
                REQUIRE_THROWS_AS([&](){
                    while (lexer.HasNext()) {
                        Token token;
                        lexer = lexer.Next(token);
                        index++;
                    }
                }(), UnhandledTokenException);

                REQUIRE(index == pair.expected_token_failure_index);
            }
        }
    }

    GIVEN("expressions that contain numeric constants with too many digits") {
        auto pair = GENERATE(values<ExpressionToFailureIndex>({
            { "label+" + std::string(13, '9'), 2 },
            { "(5--4)/%" + std::string(33, '1'), 7 },
            { "4**-0x" + std::string(9, 'F') + "5", 4 }
        }));

        WHEN("the expression is lexed") {
            THEN("InvalidNumericConstantException is thrown for the proper token") {
                ExpressionLexer lexer(pair.input_expression);

                size_t index = 0;
                REQUIRE_THROWS_AS([&](){
                    while (lexer.HasNext()) {
                        Token token;
                        lexer = lexer.Next(token);
                        index++;
                    }
                }(), InvalidNumericConstantException);

                REQUIRE(index == pair.expected_token_failure_index);
            }
        }
    }

    GIVEN("an empty expression") {
        ExpressionLexer lexer("");
        REQUIRE(!lexer.HasNext());

        Token token;
        REQUIRE_THROWS_AS(lexer.Next(token), TokensExhaustedException);
    }
}

}