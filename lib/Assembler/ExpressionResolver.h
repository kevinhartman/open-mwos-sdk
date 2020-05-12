#pragma once

#include <Expression.h>

#include <string>

namespace assembler {
    class AssemblyState;

    uint32_t ResolveExpression(const expression::Expression& expression, const AssemblyState& state);
    std::unique_ptr<expression::Expression> ParseExpression(const std::string& expr_str);
}