#include "AssemblerTarget.h"
#include "Endian.h"

#include <cstdint>
#include <tuple>
#include <memory>

namespace assembler {

class MipsAssemblerTarget : public AssemblerTarget {
public:
    MipsAssemblerTarget(support::Endian endianness);
    virtual ~MipsAssemblerTarget() override;
    support::Endian GetEndianness() override;
    Instruction EmitInstruction(const Entry& entry) override;

private:
    support::Endian endianness;
};

}