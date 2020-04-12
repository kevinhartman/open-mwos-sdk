#pragma once

#include "Assembler.h"
#include "ExpressionResolver.h" // TODO: perhaps this should be replaced by Expression(Parser|Lexer).h
#include "StringUtil.h"

#include <regex>

namespace {

using namespace assembler;

class Operand {
public:
    Operand(std::string op, std::size_t index, std::string operand, std::string debug_alias)
        : op(std::move(op)), index(index), operand(std::move(operand)), debug_alias(std::move(debug_alias)) {}

    std::string AsString() const {
        return operand;
    }

    std::unique_ptr<expression::Expression> AsExpression() const {
        try {
            return ParseExpression(operand);
        } catch (std::runtime_error& e) {
            throw OperandException(op, index, e);
        }
    }

    void Fail(std::string requirement) const {
        throw OperandException(op, index, "parameter " + debug_alias + " " + requirement);
    }

private:
    const std::string op;
    const std::size_t index;
    const std::string operand;
    const std::string debug_alias;
};

class OperandList {
public:
    OperandList(std::string op, std::vector<std::string> operands)
        : op(std::move(op)), operands(std::move(operands)) { }

    Operand Get(std::size_t index, const std::string& debug_alias) const {
        if (index >= operands.size()) {
            throw new OperandException(op, index,
                "Operation " + op + " missing positional operand " + std::to_string(index) + ".");
        }

        return Operand(op, index, operands.at(index), debug_alias);
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
    explicit Operation(const Entry& entry, const AssemblyState& state)
        : entry(entry), state(state) {}

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

    OperandList ParseOperands(std::regex delimiter = std::regex(",")) const {
        // TODO: catch split issues and return proper error
        auto parsed = Split(entry.operands.value_or(""), std::move(delimiter));
        return OperandList(entry.operation.value(), parsed);
    }

    const Entry& GetEntry() const {
        return entry;
    }

private:
    std::string BuildMessage(const std::string& requirement) const {
        return "Operation " + entry.operation.value_or("[null]") + requirement + ".";
    }

    const Entry& entry;
    const AssemblyState& state;
};

}