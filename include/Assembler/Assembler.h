#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace object {
    class ObjectFile;
}

namespace assembler {

class AssemblyState;
class AssemblerOperationHandler;
class AssemblerTarget;
class Entry;

class Assembler {

public:
    Assembler(uint16_t assembler_version, std::unique_ptr<AssemblerTarget> target);
    ~Assembler();
    std::unique_ptr<object::ObjectFile> Process(const std::vector<Entry>& listing);

protected:
    void CreateResult(AssemblyState& state);

private:
    uint16_t assembler_version;
    std::unique_ptr<AssemblerTarget> target;
    std::vector<std::unique_ptr<AssemblerOperationHandler>> op_handlers;
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
        UnexpectedEnds,
        NeedsPSectContext,
        NeedsVSectContext
    };

    OperationException(std::string op, Code code, const std::string& cause)
        : op(std::move(op)), code(code), runtime_error(GetMessage(op, cause)) {}

    std::string op;
    Code code;

private:
    static std::string GetMessage(const std::string& op, const std::string& cause) {
        return "Operation: '" + op + "', message: " + cause;
    }
};

struct OperandException : std::runtime_error {
    OperandException(std::string op, std::size_t position, const std::string& cause)
        : op(std::move(op)), position(position), runtime_error(GetMessage(op, position, cause)) {}

    OperandException(std::string op, std::size_t position, const std::runtime_error& cause)
        : op(std::move(op)), position(position), runtime_error(GetMessage(op, position, cause.what())),
        internal_exception(cause) {}

    std::optional<std::runtime_error> internal_exception {};
    std::string op;
    std::size_t position;

private:
    static std::string GetMessage(const std::string& op, std::size_t position, const std::string& cause) {
        return "Operation: '" + op + "', position: " + std::to_string(position) + ", message: " + cause;
    }
};

}