#pragma once

#include "AssemblerOperationHandler.h"

namespace assembler {

class AssemblerPseudoInstHandler : public AssemblerOperationHandler {
public:
    ~AssemblerPseudoInstHandler() override;
    bool Handle(const Entry& entry, AssemblyState& state) override;
};

}