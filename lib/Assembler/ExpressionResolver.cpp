#include "ExpressionResolver.h"

#include "AssemblyState.h"
#include <Expression.h>
#include "ExpressionLexer.h"
#include "ExpressionParser.h"

#include <functional>
#include <string>

namespace assembler {

namespace {

using namespace expression;

using ReferenceResolver = std::function<uint32_t(std::string)>;
struct ResolverVisitor : ExpressionVisitor {
    explicit ResolverVisitor(ReferenceResolver reference_resolver_func) : reference_resolver_func(reference_resolver_func) {}

    void Visit(const NumericConstantExpression& expr) override {
        result = expr.value;
    }

    void Visit(const ReferenceExpression& expr) override {
        result = reference_resolver_func(expr.Value());
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

ExpressionResolver::ExpressionResolver(const AssemblyState& state)
    : state(state) { }

uint32_t ExpressionResolver::Resolve(const Expression& expression) {
    ResolverVisitor resolver_visitor ([](auto name)-> uint32_t {
        throw "unimplemented";
    });

    expression.Accept(resolver_visitor);
    return resolver_visitor.result;
}
}