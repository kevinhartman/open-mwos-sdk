#include "ExpressionLexer.h"

#include <regex>
#include <string>
#include <vector>

namespace assembler {

constexpr auto MaxDecimalConstantLength = 12;
constexpr auto MaxHexConstantLength = 8;
constexpr auto MaxBinaryConstantLength = 32;

ExpressionLexer::ExpressionLexer(std::string expression): expression(std::move(expression)),
    lexicon({
        // Note: order matters. E.g. we must search for hex (0x...) before searching for decimal 0.
        { std::regex(R"(^(0x|\$)[A-Fa-f0-9]+)"), TokenType::HexConstant },
        { std::regex(R"(^[0-9]+)"), TokenType::DecimalConstant },
        { std::regex(R"(^%[0-1]+)"), TokenType::BinaryConstant },
        { std::regex(R"(^'.')"), TokenType::CharConstant },
        { std::regex(R"(^[A-Za-z@_][A-Za-z0-9@_$.]*)"), TokenType::SymbolicName },
        { std::regex(R"(^\.)"), TokenType::Period },
        { std::regex(R"(^-)"), TokenType::Minus },
        { std::regex(R"(^\^)"), TokenType::Hat },
        { std::regex(R"(^~)"), TokenType::Tilde },
        { std::regex(R"(^&)"), TokenType::Ampersand },
        { std::regex(R"(^!)"), TokenType::Bang },
        { std::regex(R"(^\|)"), TokenType::Pipe },
        { std::regex(R"(^\*)"), TokenType::Asterisk },
        { std::regex(R"(^/)"), TokenType::ForwardSlash },
        { std::regex(R"(^\+)"), TokenType::Plus },
        { std::regex(R"(^<)"), TokenType::LeftCarrot },
        { std::regex(R"(^>)"), TokenType::RightCarrot },
        { std::regex(R"(\()"), TokenType::LeftParen },
        { std::regex(R"(\))"), TokenType::RightParen }
    }){}

bool ExpressionLexer::HasNext() const {
    return !expression.empty();
}

ExpressionLexer ExpressionLexer::Next(assembler::Token& token) const {
    if (!HasNext()) throw TokensExhaustedException();

    for (auto token_candidate : lexicon) {
        std::smatch matches;
        if (std::regex_search(expression, matches, std::get<std::regex>(token_candidate))) {
            // Candidate matched.
            auto type = std::get<TokenType>(token_candidate);
            auto text = matches[0].str();

            // Validate numeric constants do not exceed maximum lengths.
            if (TokenType::DecimalConstant == type) {
                if (text.length() > MaxDecimalConstantLength)
                    throw InvalidNumericConstantException("Decimal constant '" + text + "' must be between 1 and 12 digits.");
            }

            if (TokenType::HexConstant == type) {
                if ((text[0] == '0' && text.length() > (2 + MaxHexConstantLength))
                || ((text[0] == '$' && text.length() > (1 + MaxHexConstantLength))))
                    throw InvalidNumericConstantException("Hexadecimal constant '" + text + "' must be between 1 and 8 digits.");
            }

            if (TokenType::BinaryConstant == type) {
                if (text.length() > 1 + (MaxBinaryConstantLength))
                    throw InvalidNumericConstantException("Binary constant '" + text + "' must be between 1 and 32 digits.");
            }

            // Write result.
            token.type = type;
            token.text = text;

            // Return new expression lexer without the matched token.
            return ExpressionLexer(
                std::regex_replace(expression, std::get<std::regex>(token_candidate), ""));
        }
    }

    throw UnhandledTokenException(expression);
}

}