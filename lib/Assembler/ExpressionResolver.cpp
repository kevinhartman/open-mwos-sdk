#include "ExpressionResolver.h"

#include "AssemblyState.h"
#include "Expression.h"
#include "ExpressionLexer.h"
#include "ExpressionParser.h"

#include <functional>

namespace assembler {

namespace {

using ReferenceResolver = std::function<uint32_t(const Expression&)>;
struct ResolverVisitor : ExpressionVisitor {
    ResolverVisitor(ReferenceResolver reference_resolver_func) : reference_resolver_func(reference_resolver_func) {}

    void Visit(const NumericConstantExpression& expr) override {
        result = expr.value;
    }

    void Visit(const ReferenceExpression& expr) override {
        result = reference_resolver_func(expr);
    }

    void Visit(const HiExpression& expr) override {
        throw "unimplemented";
    }

    void Visit(const HighExpression& expr) override {
        throw "unimplemented";
    }

    void Visit(const LoExpression& expr) override {
        throw "unimplemented";
    }

    uint32_t Resolve(const Expression& expr) {
        ResolverVisitor resolver(reference_resolver_func);
        expr.Accept(resolver);
        return resolver.result;
    }

    void Visit(const NegationExpression& expr) override {
        result = -1 * Resolve(expr.Left());
    }

    void Visit(const BitwiseNotExpression& expr) override {
        result = ~(Resolve(expr.Left()));
    }

    void Visit(const BitwiseAndExpression& expr) override {
        result = Resolve(expr.Left()) & Resolve(expr.Right());
    }

    void Visit(const BitwiseOrExpression& expr) override {
        result = Resolve(expr.Left()) | Resolve(expr.Right());
    }

    void Visit(const BitwiseXorExpression& expr) override {
        result = Resolve(expr.Left()) ^ Resolve(expr.Right());
    }

    void Visit(const MultiplicationExpression& expr) override {
        result = Resolve(expr.Left()) * Resolve(expr.Right());
    }

    void Visit(const DivisionExpression& expr) override {
        result = Resolve(expr.Left()) / Resolve(expr.Right());
    }

    void Visit(const AdditionExpression& expr) override {
        result = Resolve(expr.Left()) + Resolve(expr.Right());
    }

    void Visit(const SubtractionExpression& expr) override {
        result = Resolve(expr.Left()) - Resolve(expr.Right());
    }

    void Visit(const LogicalLeftShiftExpression& expr) override {
        result = Resolve(expr.Left()) << Resolve(expr.Right());
    }

    void Visit(const LogicalRightShiftExpression& expr) override {
        result = Resolve(expr.Left()) >> Resolve(expr.Right());
    }

    void Visit(const ArithmeticRightShiftExpression& expr) override {
        throw "unimplemented";
    }

    uint32_t result {};
    ReferenceResolver reference_resolver_func;
};
}

std::unique_ptr<Expression> ParseExpression(const std::string& expr_str) {
    auto lexer = ExpressionLexer(expr_str);
    auto parser = ExpressionParser(lexer);

    return parser.Parse();
}

uint32_t ResolveExpression(const Expression& expression, const AssemblyState& state) {
    ReferenceResolver label_resolver = [&state](const Expression& e) {
        throw "unimplemented: labels";
        return 0;
    };

    ResolverVisitor resolver(label_resolver);
    expression.Accept(resolver);

    return resolver.result;
}
}