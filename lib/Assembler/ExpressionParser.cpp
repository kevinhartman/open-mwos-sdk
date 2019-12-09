#include "ExpressionParser.h"

#include <limits>
#include <map>
#include <regex>

#include "ExpressionLexer.h"

namespace assembler {

struct PrefixParselet {
    virtual std::unique_ptr<Expression> Parse(ExpressionParser& parser, Token token) const = 0;
};

struct DecimalConstantParselet : PrefixParselet {
    // TODO: check overflow
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, Token token) const override {
        return std::make_unique<NumericConstantExpression>(std::stoul(token.text));
    }
};

struct HexConstantParselet : PrefixParselet {
    // TODO: check overflow
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, Token token) const override {
        const std::regex hex_designator(R"(^(0x|\$){1})");
        auto without_prefix = std::regex_replace(token.text, hex_designator, "");

        return std::make_unique<NumericConstantExpression>(std::stoul(without_prefix, nullptr, 16));
    }
};

struct BinaryConstantParselet : PrefixParselet {
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, Token token) const override {
        const std::regex binary_designator(R"(^\${1})");
        auto without_prefix = std::regex_replace(token.text, binary_designator, "");

        return std::make_unique<NumericConstantExpression>(std::stoul(without_prefix, nullptr, 2));
    }
};

struct CharConstantParselet : PrefixParselet {
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, Token token) const override {
        const std::regex char_constant(R"(^'(.)'$)");

        std::smatch match;
        std::regex_match(token.text, match, char_constant);
        auto extracted_char = match.str(1);

        return std::make_unique<NumericConstantExpression>(extracted_char[0]);
    }
};

struct ReferenceParselet : PrefixParselet {
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, Token token) const override {
        return std::make_unique<ReferenceExpression>(token.text);
    }
};

struct InfixParselet {
    virtual std::unique_ptr<Expression> Parse(ExpressionParser& parser, std::unique_ptr<Expression> left, Token token) const = 0;
    virtual size_t GetPrecedence() const = 0;
};

template<typename TExpression>
struct InfixOperatorParselet : InfixParselet {
    InfixOperatorParselet(size_t precedence): precedence(precedence) {}
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, std::unique_ptr<Expression> left, Token token) const override {
        return std::make_unique<TExpression>(std::move(left), std::move(parser.Parse(GetPrecedence())));
    }

    size_t GetPrecedence() const override {
        return precedence;
    }

    size_t precedence;
};

struct FuncLikeOperatorParselet : InfixParselet {
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, std::unique_ptr<Expression> left, Token token) const override {
        auto left_as_reference = dynamic_cast<ReferenceExpression*>(left.get());
        if (left_as_reference == nullptr) {
            throw "exception"; // TODO: proper exception type
        }

        std::unique_ptr<Expression> expression;
        if (left_as_reference->value == "hi") {
            expression = std::make_unique<HiExpression>(
                std::move(left), std::move(parser.Parse(GetPrecedence())));
        }

        if (left_as_reference->value == "high") {
            expression = std::make_unique<HighExpression>(
                std::move(left), std::move(parser.Parse(GetPrecedence())));
        }

        if (left_as_reference->value == "lo") {
            expression = std::make_unique<LoExpression>(
                std::move(left), std::move(parser.Parse(GetPrecedence())));
        }

        if (!expression) {
            throw "exception"; // TODO: proper exception type
        }

        parser.Consume(TokenType::RightParen);
        return expression;
    }

    size_t GetPrecedence() const override {
        // function name can never bind to something other than arg list.
        return std::numeric_limits<size_t>::max();
    }
};

template<typename TExpression>
struct PrefixOperatorParselet : PrefixParselet {
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, Token token) const override {
        return std::make_unique<TExpression>(parser.Parse(0)); // TODO: is prefix precedence needed?
    }
};

struct GroupParselet : PrefixParselet {
    std::unique_ptr<Expression> Parse(ExpressionParser& parser, Token token) const override {
        auto expression = std::move(parser.Parse(0)); // reset precedence
        parser.Consume(TokenType::RightParen);

        return expression;
    }
};

// note: these are only shared_ptr because init from initializer list requires copy, which cannot work with unique_ptrs
const std::map<TokenType, std::shared_ptr<PrefixParselet>> prefix_parslets {
    { DecimalConstant, std::make_shared<DecimalConstantParselet>() },
    { HexConstant, std::make_shared<HexConstantParselet>() },
    { BinaryConstant, std::make_shared<BinaryConstantParselet>() },
    { CharConstant, std::make_shared<CharConstantParselet>() },
    { SymbolicName, std::make_shared<ReferenceParselet>() },
    { Period, std::make_shared<ReferenceParselet>() },
    { Asterisk, std::make_shared<ReferenceParselet>() },

    { Minus, std::make_shared<PrefixOperatorParselet<NegationExpression>>() },
    { Tilde, std::make_shared<PrefixOperatorParselet<BitwiseNotExpression>>() },
    { Hat, std::make_shared<PrefixOperatorParselet<BitwiseNotExpression>>() },

    { LeftParen, std::make_shared<GroupParselet>() }
};

// Ordered by precedence for readability.
const std::map<TokenType, std::shared_ptr<InfixParselet>> infix_parslets {
    { LeftParen,         std::make_shared<FuncLikeOperatorParselet>() },
    { Ampersand,         std::make_shared<InfixOperatorParselet<BitwiseAndExpression>>(18) },
    { Bang,              std::make_shared<InfixOperatorParselet<BitwiseOrExpression>>(17) },
    { Pipe,              std::make_shared<InfixOperatorParselet<BitwiseOrExpression>>(17) },
    { Hat,               std::make_shared<InfixOperatorParselet<BitwiseXorExpression>>(16) },
    { Asterisk,          std::make_shared<InfixOperatorParselet<MultiplicationExpression>>(15) },
    { ForwardSlash,      std::make_shared<InfixOperatorParselet<DivisionExpression>>(14) },
    { Plus,              std::make_shared<InfixOperatorParselet<AdditionExpression>>(13) },
    { Minus,             std::make_shared<InfixOperatorParselet<SubtractionExpression>>(12) },
    { DoubleLeftCarrot,  std::make_shared<InfixOperatorParselet<LogicalLeftShiftExpression>>(11) },
    { DoubleRightCarrot, std::make_shared<InfixOperatorParselet<LogicalRightShiftExpression>>(10) }
    // TODO: there appears to be an "arithmetic" right shift supported by OS9 expression trees in ROF15,
    //       but there isn't a documented operator for this. Perhaps it's ">>>"?

};

std::optional<size_t> GetInfixPrecedence(TokenType token_type) {
    auto infix_iterator = infix_parslets.find(token_type);
    if (infix_iterator == infix_parslets.end()) {
        return std::nullopt;
    }

    return infix_iterator->second->GetPrecedence();
}

ExpressionParser::ExpressionParser(ExpressionLexer lexer) : lexer(std::move(lexer)) {}

std::unique_ptr<Expression> ExpressionParser::Parse() {
    return Parse(0);
}

std::unique_ptr<Expression> ExpressionParser::Parse(size_t precedence) {
    if (!lexer.HasNext()) {
        throw "exception"; // TODO: replace
    }

    Token next_token;
    lexer = lexer.Next(next_token);

    auto prefix_iterator = prefix_parslets.find(next_token.type);
    if (prefix_iterator == prefix_parslets.end()) {
        // Couldn't find suitable prefix parselet.
        throw "exception"; // TODO: replace
    }

    const PrefixParselet& prefix_parselet = *prefix_iterator->second;
    auto left_expression = prefix_parselet.Parse(*this, next_token);

    if (lexer.HasNext()) {
        auto infix_lexer = lexer.Next(next_token);
        while(precedence < GetInfixPrecedence(next_token.type).value_or(0)) {
            // We are parsing an infix expression, so we keep the advanced lexer.
            lexer = infix_lexer;

            // Get the infix parselet and invoke it.
            const InfixParselet& infix_parselet = *infix_parslets.at(next_token.type);
            left_expression = infix_parselet.Parse(*this, std::move(left_expression), next_token);

            // If no tokens remain, we're done.
            if (!lexer.HasNext()) break;

            // Update next_token.
            infix_lexer = lexer.Next(next_token);
        }
    }

    return left_expression;
}

void ExpressionParser::Consume(TokenType expected_token_type) {
    if (!lexer.HasNext()) {
        throw "exception"; // TODO: proper exception type
    }

    Token token;
    auto lexer_next = lexer.Next(token);

    if (token.type != expected_token_type) {
        throw "exception"; // TODO: proper exception type
    }

    lexer = lexer_next;
}

}