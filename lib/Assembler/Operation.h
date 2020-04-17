#pragma once

#include "Assembler.h"
#include "ExpressionResolver.h" // TODO: perhaps this should be replaced by Expression(Parser|Lexer).h
#include <ExpressionLexer.h>
#include <ExpressionParser.h>
#include "StringUtil.h"

#include <memory>
#include <regex>

namespace {

using namespace assembler;

struct OperandInfo {
    const std::string op;
    const std::size_t index;
    const std::string operand;
    const std::string debug_alias;
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

    u_int32_t Resolve(const ExpressionResolver& resolver) {
        //resolver.Resolve(*expr, )
        throw OperandException(info.op, info.index, "expression resolution unimplemented");
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

    OperandList ParseOperands(const std::regex& delimiter = std::regex(",")) const {
        // TODO: catch split issues and return proper error
        auto parsed = Split(entry.operands.value_or(""), delimiter);
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