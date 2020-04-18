#pragma once

#include <Expression.h>
#include <AssemblerTarget.h>
#include <ObjectFile.h>

namespace expression {
    bool operator==(const expression::Expression &e1, const expression::Expression &e2);
}

namespace assembler {
    bool operator==(const ExpressionMapping &e1, const ExpressionMapping &e2);
}

namespace object {
    bool operator==(const SymbolInfo &s1, const SymbolInfo &s2);
}