#pragma once

#include "Assembler.h"
#include "ExpressionResolver.h" // TODO: perhaps this should be replaced by Expression(Parser|Lexer).h
#include "StringUtil.h"

#include <regex>

namespace {

using namespace assembler;

class Operand {
public:
    Operand(std::string op, std::size_t index, std::string operand)
        : op(std::move(op)), operand(std::move(operand)) {}

    std::string AsString() {
        return operand;
    }

    std::unique_ptr<expression::Expression> AsExpression() {
        try {
            return ParseExpression(operand);
        } catch (std::runtime_error& e) {
            throw OperandException(op, index, e);
        }
    }

    void Fail(std::string requirement) {
        throw OperandException(op, index, requirement);
    }

private:
    std::string op;
    std::size_t index;
    const std::string operand;
};

class OperandList {
public:
    OperandList(std::string op, std::vector<std::string> operand_strings) : op(std::move(op)) {
        operands.reserve(operand_strings.size());

        for (std::size_t i = 0; i < operand_strings.size(); i++) {
            operands.emplace_back(Operand(op, i, operand_strings.at(i)));
        }
    }

    Operand Get(std::size_t index) const {
        if (index >= operands.size()) {
            throw new OperandException(op, index,
                "Operation " + op + " missing positional operand " + std::to_string(index) + ".");
        }

        return operands.at(index);
    }

    std::size_t Count() const {
        return operands.size();
    }

private:
    std::vector<Operand> operands {};
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