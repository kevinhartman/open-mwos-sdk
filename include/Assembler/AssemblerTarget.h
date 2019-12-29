#pragma once

#include <tuple>
#include <memory>
#include <vector>

#include "Expression.h"

namespace assembler {
class Entry;

struct ExpressionMapping {
    size_t offset;
    size_t bit_count;
    std::shared_ptr<Expression> expression;
};

struct Instruction {
    uint64_t data;
    size_t size;
    std::vector<ExpressionMapping> expr_mappings;
};

class AssemblerTarget {
public:
    virtual ~AssemblerTarget() = default;
    virtual Instruction EmitInstruction(const Entry& entry) = 0;
};

}