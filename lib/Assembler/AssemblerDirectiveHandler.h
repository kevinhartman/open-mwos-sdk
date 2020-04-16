#pragma once

#include "AssemblerOperationHandler.h"

namespace assembler {

class AssemblerDirectiveHandler : public AssemblerOperationHandler {
public:
    ~AssemblerDirectiveHandler() override;
    bool Handle(const Entry& entry, AssemblyState& state) override;
};

}