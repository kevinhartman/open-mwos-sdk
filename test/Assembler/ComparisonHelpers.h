#pragma once

#include <Expression.h>
#include <AssemblyState.h>
#include <ObjectFile.h>

namespace expression {
    bool operator==(const expression::Expression &e1, const expression::Expression &e2);
}

namespace object {
bool operator==(const ExpressionMapping &e1, const ExpressionMapping &e2);
bool operator==(const SymbolInfo &s1, const SymbolInfo &s2);
}