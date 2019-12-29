#pragma once

#include "Expression.h"

namespace assembler {
    bool operator==(const Expression &e1, const Expression &e2);

    class ExpressionMapping;
    bool operator==(const ExpressionMapping &e1, const ExpressionMapping &e2);
}
