#pragma once

#include <Expression.h>
#include <AssemblerTarget.h>

#include <ostream>

namespace expression {
std::ostream &operator<<(std::ostream &os, const Expression &expr);
}

namespace assembler {
std::ostream &operator<<(std::ostream &os, const ExpressionMapping &expr);
}