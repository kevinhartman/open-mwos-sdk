#pragma once

#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace object {
    class ObjectFile;
}

namespace assembler {

class AssemblyState;
class AssemblerTarget;
class Entry;

class Assembler {

public:
    Assembler(uint16_t assembler_version, std::unique_ptr<AssemblerTarget> target);
    ~Assembler();
    std::unique_ptr<object::ObjectFile> Process(const std::vector<Entry>& listing);

protected:
    bool HandleDirective(const Entry& entry, AssemblyState& state);
    bool HandlePseudoInstruction(const Entry& entry, AssemblyState& state);
    std::unique_ptr<object::ObjectFile> CreateResult(AssemblyState& state);

private:
    uint16_t assembler_version;
    std::unique_ptr<AssemblerTarget> target;
};

enum class OperationId {
    Equ
};

struct OperationException : std::runtime_error {
    enum class Code {
        MissingLabel,
        UnexpectedOperands,
        TooManyOperands,
        UnexpectedLabel,
        UnexpectedComment,
        DuplicateSymbol,
        UnexpectedPSect,
        UnexpectedVSect,
    };

    OperationException(OperationId op, Code code, const std::string& cause)
        : runtime_error(cause), op(op), code(code) {}

    OperationId op;
    Code code;
};

struct OperandException : std::runtime_error {
    OperandException(OperationId op, std::size_t position, const std::string& cause)
        : op(op), position(position), runtime_error(cause) {}

    OperandException(OperationId op, std::size_t position, const std::runtime_error& cause)
        : op(op), position(position), runtime_error(cause.what()), internal_exception(cause) {}

    std::optional<std::runtime_error> internal_exception {};
    OperationId op;
    std::size_t position;
};

}