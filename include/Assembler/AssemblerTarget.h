#pragma once

#include <tuple>
#include <memory>
#include <vector>

#include "Expression.h"
#include "Endian.h"

namespace assembler {
class Entry;

struct ExpressionMapping {
    size_t offset;
    size_t bit_count;
    std::shared_ptr<Expression> expression;
};

struct Instruction {
    union {
        uint8_t raw[sizeof(uint64_t)];
        uint64_t u64;
        uint32_t u32;
        uint16_t u16;
        uint8_t u8;
    } data;
    size_t size;
    std::vector<ExpressionMapping> expr_mappings;
};

class AssemblerTarget {
public:
    virtual ~AssemblerTarget() = default;
    virtual support::Endian GetEndianness() = 0;
    virtual Instruction EmitInstruction(const Entry& entry) = 0;
    // TODO: PatchInstruction() for applying expression result to instruction? Perhaps not needed.
};

}