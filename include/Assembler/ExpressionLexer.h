#pragma once

#include <exception>
#include <string>
#include <utility>
#include <tuple>
#include <regex>

namespace assembler {

enum TokenType {
    DecimalConstant,
    HexConstant,
    BinaryConstant,
    CharConstant,
    SymbolicName,

    Period,
    Minus,
    Hat,
    Tilde,
    Ampersand,
    Bang,
    Pipe,
    Asterisk,
    ForwardSlash,
    Plus,
    LeftCarrot,
    RightCarrot,

    LeftParen,
    RightParen
};

struct Token {
    TokenType type;
    std::string text;
};

class ExpressionLexer {
public:
    explicit ExpressionLexer(std::string expression);
    ExpressionLexer Next(Token& token) const;
    bool HasNext() const;

private:
    std::string expression;
    std::vector<std::tuple<std::regex, TokenType>> lexicon;
};

struct UnhandledTokenException : std::runtime_error {
    explicit UnhandledTokenException(const std::string& token) : runtime_error("Unhandled token at start of expression: " + token) {}
};

struct TokensExhaustedException : std::runtime_error {
    TokensExhaustedException() : runtime_error("The expression has no token.") {}
};

struct InvalidNumericConstantException : std::runtime_error {
    explicit InvalidNumericConstantException(const std::string& message) : runtime_error(message) {}
};

}
