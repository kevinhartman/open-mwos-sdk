#pragma once

#include <string>

namespace assembler {

class Expression;

class ExpressionParser {
public:
    ExpressionParser() = default;
    bool Parse(const std::string& expression, Expression& out);
};

}