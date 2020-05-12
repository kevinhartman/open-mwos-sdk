#include "ComparisonHelpers.h"

#include <Expression.h>
#include <ObjectFile.h>

#include <cassert>
#include <typeinfo>

namespace expression {

bool operator==(const expression::Expression &e1, const expression::Expression &e2) {
    if (typeid(e1) != typeid(e2)) {
        return false;
    }

    // Handle prefix case.
    const auto *e1_prefix = dynamic_cast<const PrefixExpression *>(&e1);
    if (e1_prefix != nullptr) {
        return e1_prefix->Left() == dynamic_cast<const PrefixExpression *>(&e2)->Left();
    }

    // Handle infix case.
    const auto *e1_infix = dynamic_cast<const InfixExpression *>(&e1);
    if (e1_infix != nullptr) {
        return e1_infix->Left() == dynamic_cast<const InfixExpression *>(&e2)->Left()
            && e1_infix->Right() == dynamic_cast<const InfixExpression *>(&e2)->Right();
    }

    // Handle numeric constant.
    const auto *e1_number = dynamic_cast<const ValueExpression <uint32_t> *>(&e1);
    if (e1_number != nullptr) {
        return e1_number->Value() == dynamic_cast<const ValueExpression <uint32_t> *>(&e2)->Value();
    }

    // Handle string constant.
    const auto *e1_string = dynamic_cast<const ValueExpression <std::string> *>(&e1);
    if (e1_string != nullptr) {
        return e1_string->Value() == dynamic_cast<const ValueExpression <std::string> *>(&e2)->Value();
    }

    assert(false); // unhandled expression type!
    return false;
}

}

namespace assembler {
bool operator==(const ExpressionMapping &e1, const ExpressionMapping &e2) {
    return std::tie(e1.offset, e1.bit_count, *e1.expression) == std::tie(e2.offset, e2.bit_count, *e2.expression);
}
}

namespace object {
bool operator==(const SymbolInfo &s1, const SymbolInfo &s2) {
    return std::tie(s1.type, s1.is_global, s1.value.value()) == std::tie(s2.type, s2.is_global, s2.value.value());
}
}