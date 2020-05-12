#pragma once

#include <tuple>
#include <memory>
#include <vector>

#include <Expression.h>
#include "Endian.h"

namespace object {
    class ObjectFile;
}

namespace assembler {

struct ExpressionMapping {
    size_t offset;
    size_t bit_count;
    std::shared_ptr<expression::Expression> expression;
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

class Entry;

class AssemblerTarget {
public:
    virtual ~AssemblerTarget() = default;
    virtual Instruction EmitInstruction(const Entry&) = 0;
    // TODO: PatchInstruction() for applying expression result to instruction? Perhaps not needed.
    virtual void SetTargetSpecificProperties(object::ObjectFile&) = 0;

};

}