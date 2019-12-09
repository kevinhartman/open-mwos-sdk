#pragma once

#include "Expression.h"
#include "ExpressionLexer.h"

#include <map>
#include <string>

namespace assembler {

class ExpressionParser {
public:
    explicit ExpressionParser(ExpressionLexer lexer);
    std::unique_ptr<Expression> Parse();
    std::unique_ptr<Expression> Parse(size_t precedence);
    void Consume(TokenType expected_token_type);

private:
    ExpressionLexer lexer;
};

}