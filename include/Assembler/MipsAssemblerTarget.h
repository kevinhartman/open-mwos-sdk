#include "AssemblerTarget.h"
#include "Endian.h"

#include <cstdint>
#include <tuple>
#include <memory>

namespace assembler {

class MipsAssemblerTarget : public AssemblerTarget {
public:
    explicit MipsAssemblerTarget(support::Endian);
    ~MipsAssemblerTarget() override;
    Instruction EmitInstruction(const Entry&) override;
    void SetTargetSpecificProperties(object::ObjectFile&) override;

private:
    support::Endian endianness;
};

}