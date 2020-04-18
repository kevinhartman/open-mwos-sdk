#pragma once

#include <Expression.h>

#include <string>

namespace assembler {
class AssemblyState;

class ExpressionResolver {
public:
    explicit ExpressionResolver(const AssemblyState&);

    uint32_t Resolve(const expression::Expression& expression) const;

private:
    // TODO: we should probably be using shared pointer to avoid worrying about lifetimes.
    const AssemblyState& state;
};

}