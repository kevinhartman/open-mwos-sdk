#pragma once

#include "Expression.h"

#include <ostream>

namespace assembler {
std::ostream &operator<<(std::ostream &os, const Expression &expr);

class ExpressionMapping;
std::ostream &operator<<(std::ostream &os, const ExpressionMapping &expr);

}