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

class Entry;
class AssemblerOperationHandler;

class AssemblerTarget {
public:
    virtual ~AssemblerTarget() = default;
    virtual void SetTargetSpecificProperties(object::ObjectFile&) = 0;
    virtual std::unique_ptr<AssemblerOperationHandler> GetOperationHandler() = 0;
};

}