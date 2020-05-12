#pragma once

#include <tuple>
#include <memory>
#include <vector>

#include <Expression.h>
#include <ObjectFile.h>
#include "Endian.h"

namespace assembler {
class Entry;

class AssemblerTarget {
public:
    virtual ~AssemblerTarget() = default;
    virtual support::Endian GetEndianness() = 0;
    virtual object::Instruction EmitInstruction(const Entry& entry) = 0;
    // TODO: PatchInstruction() for applying expression result to instruction? Perhaps not needed.
};

}