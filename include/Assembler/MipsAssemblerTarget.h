#include "AssemblerTarget.h"

#include <cstdint>
#include <tuple>
#include <memory>

namespace assembler {

class MipsAssemblerTarget : public AssemblerTarget {
public:
    MipsAssemblerTarget();
    virtual ~MipsAssemblerTarget();
    Instruction EmitInstruction(const Entry& entry) override;
};

}