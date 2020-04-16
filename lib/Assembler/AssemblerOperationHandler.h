#pragma once

#include <AssemblerTypes.h>
#include "AssemblyState.h"

namespace assembler {

class AssemblerOperationHandler {
public:
    virtual ~AssemblerOperationHandler() = default;
    virtual bool Handle(const Entry& entry, AssemblyState& state) = 0;
};

}