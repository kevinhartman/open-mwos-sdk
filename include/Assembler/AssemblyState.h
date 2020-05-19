#pragma once

#include "AssemblerTarget.h"
#include "AssemblerTypes.h"
#include "Operation.h"
#include <Expression.h>
#include <ObjectFile.h>

#include <functional>
#include <map>
#include <optional>
#include <vector>

namespace assembler {

class AssemblyState;
class SecondPassAction {
private:
    std::vector<std::unique_ptr<Operand>> operands {};
    std::vector<std::unique_ptr<ExpressionOperand>> expr_operands {};
    std::function<void(AssemblyState&)> _action;

    Operand& Accept(std::unique_ptr<Operand> operand) {
        return *operands.emplace_back(std::move(operand));
    }

    ExpressionOperand& Accept(std::unique_ptr<ExpressionOperand> operand) {
        return *expr_operands.emplace_back(std::move(operand));
    }

    template <typename F, typename ...T>
    void InitAction(F action, const T& ...ops) {
        _action = [action, &ops...](AssemblyState& state) {
            action(state, ops...);
        };
    }

public:
    template <typename F, typename ...T>
    explicit SecondPassAction(F action, std::unique_ptr<T> ...ops) {
        // Note that Accept has a side effect. It moves the op ptr into this class,
        // saving it into the relevant vector (depending on if it's an Operand
        // or an ExpressionOperand).
        // A reference to the op is then forwarded into Init.
        InitAction(action, this->Accept(std::move(ops))...);
    }

    void operator()(AssemblyState& state) {
        _action(state);
    }
};

struct ExpressionMapping {
    size_t offset;
    size_t bit_count;
    std::shared_ptr<expression::Expression> expression;
};

struct MemoryValue {
    union {
        uint8_t raw[sizeof(uint64_t)];
        uint64_t u64;
        uint32_t u32;
        uint16_t u16;
        uint8_t u8;
    } data {};
    size_t size {};
    bool is_signed {};
    std::vector<ExpressionMapping> expr_mappings {};
};

//struct EquDefinition {
//    std::unique_ptr<expression::Expression> value;
//};

struct SetDefinition {
    std::unique_ptr<expression::Expression> value;
};

struct VSect {
    bool isRemote;
};

typedef size_t local_offset;
struct PSect {
    std::vector<VSect> vsects {};

    std::map<local_offset, MemoryValue> initialized_data {};
    std::map<local_offset, MemoryValue> remote_initialized_data {};
    std::map<local_offset, MemoryValue> code_data {};

    std::map<std::string, object::SymbolInfo> symbols {};

    std::map<std::string, std::unique_ptr<ExpressionOperand>> equs {};
};

struct AssemblyState {

    inline auto& GetInitDataCounter() {
        return !in_vsect
            ? result->counter.code
            : in_remote_vsect
                ? result->counter.remote_initialized_data
                : result->counter.initialized_data;
    }

    inline auto& GetInitDataMap() {
        return !in_vsect
            ? psect.code_data
            : in_remote_vsect
                ? psect.remote_initialized_data
                : psect.initialized_data;
    }

    void DeferToSecondPass(std::unique_ptr<SecondPassAction> action) {
        second_pass_queue2.emplace_back(std::move(action));
    }

    inline std::optional<object::SymbolInfo> GetSymbol(std::string name) const {
        auto symbol_itr = psect.symbols.find(name);
        if (symbol_itr != psect.symbols.end()) {
            return symbol_itr->second;
        }

        return std::nullopt;
    }

    inline void UpdateSymbol(const Label& label, const object::SymbolInfo& symbol_info) {
        symbol_name_to_label[label.name] = label;
        psect.symbols[label.name] = symbol_info;
    }

    bool found_program_end = false;
    bool found_psect = false;

    bool in_psect = false;
    bool in_vsect = false;
    bool in_remote_vsect = false;
    std::vector<Label> pending_labels {};
    std::map<std::string, Label> symbol_name_to_label;

    PSect psect {};
    std::vector<VSect> root_vsects {};

    // TODO: this design raises a few interesting considerations. For example:
    //   - Counter values (probably among other things) should only be allowed to be manipulated in the first pass.
    //     What if instead of making the state accessible to both passes, we only allow the second pass write access
    //     to the result object file, and only allow the first pass write access to the state?
    std::vector<std::unique_ptr<SecondPassAction>> second_pass_queue2 {};

    std::unique_ptr<object::ObjectFile> result = std::make_unique<object::ObjectFile>();
};
}