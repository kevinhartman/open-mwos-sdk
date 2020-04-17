#pragma once

#include <Expression.h>

#include <string>

namespace assembler {
class AssemblyState;

class ExpressionResolver {
public:
    ExpressionResolver(const AssemblyState&);

    uint32_t Resolve(const expression::Expression& expression);

private:
    // TODO: we should probably be using shared pointer to avoid worrying about lifetimes.
    const AssemblyState& state;
};

// TODO: find a better place for this
std::unique_ptr<expression::Expression> ParseExpression(const std::string& expr_str);

}