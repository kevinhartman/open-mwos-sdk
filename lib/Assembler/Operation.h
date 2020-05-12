#pragma once

#include "Assembler.h"
#include "ExpressionResolver.h" // TODO: perhaps this should be replaced by Expression(Parser|Lexer).h
#include <ExpressionLexer.h>
#include <ExpressionParser.h>
#include "StringUtil.h"

#include <memory>
#include <regex>
#include <sstream>

namespace {

using namespace assembler;

struct OperandInfo {
    std::string op {};
    std::size_t index {};
    std::string operand {};
    std::string debug_alias {};
};

class Operand {
public:
    Operand(OperandInfo info) : info(std::move(info)) {}

    std::string AsString() const {
        return info.operand;
    }

    //    ReferenceResolver label_resolver = [&state](const std::string& e)-> uint32_t {
    //        // TODO: not implemented.
    //        throw UnresolvedNameException(e);
    //    };


    void Fail(std::string requirement) const {
        throw OperandException(info.op, info.index, "parameter " + info.debug_alias + " " + requirement);
    }

protected:
    OperandInfo info;
};


class ExpressionOperand : public Operand {
public:
    explicit ExpressionOperand(const OperandInfo& info) : Operand(info) {
        try {
            auto lexer = ExpressionLexer(info.operand);
            auto parser = ExpressionParser(lexer);
            expr = parser.Parse();
        } catch (std::runtime_error& e) {
            throw OperandException(info.op, info.index, e);
        }
    }

    u_int32_t Resolve(const ExpressionResolver& resolver) const {
        try {
            return resolver.Resolve(*expr);
        } catch (std::runtime_error& ex) {
            // TODO: improve error message
            throw OperandException(info.op, info.index, "failed to resolve expression.");
        }

    }

private:
    std::unique_ptr<expression::Expression> expr;
};

class OperandList {
private:
    template <typename T>
    inline auto Get(std::size_t index, const std::string& debug_alias) const {
        if (index >= operands.size()) {
            throw new OperandException(op, index,
                "Operation " + op + " missing positional operand " + std::to_string(index) + ".");
        }

        return std::make_unique<T>(OperandInfo { op, index, operands.at(index), debug_alias});
    }

public:
    OperandList(std::string op, std::vector<std::string> operands)
        : op(std::move(op)), operands(std::move(operands)) { }

    std::unique_ptr<ExpressionOperand> GetExpression(std::size_t index, const std::string& debug_alias) {
        return Get<ExpressionOperand>(index, debug_alias);
    }

    std::unique_ptr<Operand> Get(std::size_t index, const std::string& debug_alias) const {
        return Get<Operand>(index, debug_alias);
    }

    std::size_t Count() const {
        return operands.size();
    }

private:
    std::vector<std::string> operands {};
    std::string op;
};

class Operation {
public:
    static constexpr auto SplitOnCommaRespectingStrings = [](const std::string& operands) {
        auto split_by_comma = Split(operands, std::regex(","));

        std::vector<std::string> result {};
        result.reserve(split_by_comma.size());

        std::stringstream ss {};
        for (const std::string& segment : split_by_comma) {
            std::size_t pos {};
            if (ss.tellp() == ss.beg) {
                // we're not lexing a string yet.
                if (segment.front() == '"') {
                    // we're entering a string
                    pos = segment.find('"', 1);
                } else {
                    // this must be an expression or empty string, so we're done.
                    result.emplace_back(segment);
                    continue;
                }
            } else {
                // we're already in a string. Find the terminating "
                pos = segment.find('"');
            }

            if (pos == std::string::npos) {
                // not found, so the comma was supposed to be in the string
                ss << segment << ",";
            } else if (pos == segment.length() - 1) {
                // end of string was found
                ss << segment;
                const auto& quoted = ss.str();
                result.emplace_back(quoted);

                // reset ss
                std::stringstream new_ss {};
                ss.swap(new_ss);
            } else {
                // matching " appeared before the end of the string
                throw std::runtime_error("invalid character string");
            }
        }

        return result;
    };

    static constexpr auto SplitOnComma = [](auto&& operands) {
        return Split(operands, std::regex(","));
    };

    explicit Operation(const Entry& entry)
        : entry(entry) {}

    void RequireLabel() const {
        if (!entry.label)
            throw OperationException(entry.operation.value(), OperationException::Code::MissingLabel,
                BuildMessage("must have a label"));
    }

    void RequireNoOperands() const {
        if (entry.operands)
            throw OperationException(entry.operation.value(), OperationException::Code::UnexpectedOperands,
                BuildMessage("cannot have operands"));
    }

    void RequireNoLabel() const {
        if (entry.label)
            throw OperationException(entry.operation.value(), OperationException::Code::UnexpectedLabel,
                BuildMessage("cannot have a label"));
    }

    void RequireNoComment() const {
        if (entry.comment)
            throw OperationException(entry.operation.value(), OperationException::Code::UnexpectedComment,
                BuildMessage("cannot have a comment."));
    }

    void Fail(OperationException::Code code, std::string requirement) const {
        throw OperationException(entry.operation.value(), code, BuildMessage(requirement));
    }

    OperandList ParseOperands(const std::function<std::vector<std::string>(std::string)>& split_func) const {
        // TODO: catch split issues and return proper error
        auto parsed = split_func(entry.operands.value_or(""));
        return OperandList(entry.operation.value(), parsed);
    }

    OperandList ParseOperands() const {
        return ParseOperands(SplitOnComma);
    }

    const Entry& GetEntry() const {
        return entry;
    }

private:
    std::string BuildMessage(const std::string& requirement) const {
        return "Operation " + entry.operation.value_or("[null]") + requirement + ".";
    }

    const Entry& entry;
};

}