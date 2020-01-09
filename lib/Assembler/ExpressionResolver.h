#pragma once

#include "Expression.h"

#include <string>

namespace assembler {
    class AssemblyState;

    uint32_t ResolveExpression(const Expression& expression, const AssemblyState& state);
    std::unique_ptr<Expression> ParseExpression(const std::string& expr_str);
}