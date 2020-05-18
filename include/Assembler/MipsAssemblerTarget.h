#include "AssemblerOperationHandler.h"
#include "Endian.h"

#include <cstdint>
#include <tuple>
#include <memory>

namespace assembler {

class MipsAssemblerTarget : public AssemblerTarget {
public:
    explicit MipsAssemblerTarget(support::Endian);
    ~MipsAssemblerTarget() override;

    void SetTargetSpecificProperties(object::ObjectFile&) override;
    std::unique_ptr<AssemblerOperationHandler> GetOperationHandler() override;
private:
    support::Endian endianness;
};

}