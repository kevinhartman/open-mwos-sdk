#pragma once

#include <Expression.h>
#include <AssemblyState.h>
#include <ObjectFile.h>

#include <ostream>

namespace expression {
std::ostream &operator<<(std::ostream &os, const Expression &expr);
}

namespace assembler {
std::ostream &operator<<(std::ostream &os, const ExpressionMapping &expr);
}

namespace object {
std::ostream &operator<<(std::ostream &os, const SymbolInfo &sym);
}