#pragma once

#include "../Expression/Expression.h"
#include "ExpressionLexer.h"

#include <map>
#include <string>

namespace assembler {

class ExpressionParser {
public:
    explicit ExpressionParser(ExpressionLexer lexer);
    std::unique_ptr<expression::Expression> Parse();
    std::unique_ptr<expression::Expression> Parse(size_t precedence);
    void Consume(TokenType expected_token_type);

private:
    size_t GetCurrentTokenIndex() const;
    std::string PrintCurrentExprContext() const;

private:
    ExpressionLexer lexer;
    std::string initial_expr_string;
};

struct ExpectedExpressionException : std::runtime_error {
    ExpectedExpressionException(const std::string& got_instead, const std::string& context)
        : runtime_error("Expected expression. Got: '" + got_instead + "'. Context:\n" + context) {}
};

struct ExpectedTokenException : std::runtime_error {
    ExpectedTokenException(const std::string& expected, const std::string& got_instead, const std::string& context)
        : runtime_error("Expected token: '" + expected + "' Got: '" + got_instead + "'. Context:\n" + context) {}
};

struct NumericExpressionOutOfRangeException : std::runtime_error {
    NumericExpressionOutOfRangeException(const std::string& expression)
        : runtime_error("Value cannot be represented in 32 bits: '" + expression + "'.") {}
};

struct InvalidFuncLikeOperatorException : std::runtime_error {
    InvalidFuncLikeOperatorException(const std::string& got_instead)
        : runtime_error("Invalid function name. Valid functions are 'hi', 'lo' and 'high'. Got: '" + got_instead + "'.") {}
};

}