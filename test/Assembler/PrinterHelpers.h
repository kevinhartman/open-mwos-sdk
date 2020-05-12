#pragma once

#include <Expression.h>
#include <ObjectFile.h>

#include <ostream>

namespace expression {
std::ostream &operator<<(std::ostream &os, const Expression &expr);
}

namespace object {
std::ostream &operator<<(std::ostream &os, const ExpressionMapping &expr);
}