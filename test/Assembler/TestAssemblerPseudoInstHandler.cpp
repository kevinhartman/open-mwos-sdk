#include <catch2/catch.hpp>

#include <AssemblerPseudoInstHandler.h>

#include <Assembler.h>
#include <AssemblerTypes.h>
#include <InputFileParser.h>

#include <AssemblyState.h>

#include <ObjectFile.h>

#include "ComparisonHelpers.h"
#include "PrinterHelpers.h"

#include <memory>
#include <sstream>

namespace assembler {

/**
 * Catch2 Matcher class used to determine if an OperationException has the expected
 * code.
 */
class AssemblerOperationExceptionCode : public Catch::MatcherBase<OperationException> {
    OperationException::Code code;
public:
    explicit AssemblerOperationExceptionCode(OperationException::Code code) : code(code) {}

    // Performs the test for this matcher
    bool match(const OperationException& ex) const override {
        return ex.code == code;
    }

    std::string describe() const override {
        std::ostringstream ss;
        ss << "has reason code " << static_cast<int>(code);
        return ss.str();
    }
};

// Eg.) REQUIRE_THROWS_MATCHES(handler.Handle(entry, state), OperationException, IsOperationFailureReason(OperationException::Code::UnexpectedPSect));
inline AssemblerOperationExceptionCode IsOperationFailureReason(OperationException::Code code) {
    return AssemblerOperationExceptionCode(code);
}

namespace {
Entry ParseEntry(const std::string& input_line) {
    std::stringstream input{};
    input << input_line << std::endl;

    assembler::InputFileParser parser {};
    parser.Parse(input);

    return parser.GetListing().front();
}
}
SCENARIO("DS operation expression resolution behavior", "[assembler]") {
    AssemblerPseudoInstHandler handler {};

    GIVEN("state in a vsect and psect context") {
        AssemblyState state {};
        state.in_psect = true;
        state.in_vsect = true;

        WHEN("the size operand is a constant expression") {
            auto entry = ParseEntry("var ds.b 4+5/2");

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.uninitialized_data == 6);
            REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }
        AND_WHEN("the size operand is an expression involving an external reference") {
            auto entry = ParseEntry("var ds.b external+1");

            REQUIRE_THROWS_AS(handler.Handle(entry, state), OperandException);
        }
        AND_WHEN("the size operand is an expression involving a constant expression EQU") {
            OperandInfo info {};
            info.op = "equ";
            info.operand = "5+2";

            state.equs["constequ"] = std::make_unique<ExpressionOperand>(info);

            auto entry = ParseEntry("var ds.b constequ+1");
            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.uninitialized_data == 8);
            REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }
        AND_WHEN("the size operand is an expression involving an EQU that references an external name") {
            OperandInfo info {};
            info.op = "equ";
            info.operand = "5+extern";

            state.equs["equ"] = std::make_unique<ExpressionOperand>(info);

            auto entry = ParseEntry("var ds.b equ+1");
            REQUIRE_THROWS_AS(handler.Handle(entry, state), OperandException);
        }
    }
}

SCENARIO("DS operation behavior", "[assembler]") {
    AssemblerPseudoInstHandler handler {};

    GIVEN("state in a vsect and psect context") {
        AssemblyState state {};
        state.in_psect = true;
        state.in_vsect = true;

        WHEN("a single byte is declared with the label 'var'") {
            auto entry = ParseEntry("var ds.b 1");

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.uninitialized_data == 1);
            REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }
        AND_WHEN("two bytes are declared with the label 'var'") {
            auto entry = ParseEntry("var ds.b 2");

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.uninitialized_data == 2);
            REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }
        AND_WHEN("0 bytes are declared with the label 'var'") {
            auto entry = ParseEntry("var ds.b 0");

            REQUIRE_THROWS_AS(handler.Handle(entry, state), OperandException);
        }
    }
}

SCENARIO("DC operation behavior", "[assembler]") {
    AssemblerPseudoInstHandler handler {};

    GIVEN("multiple unsigned constant bytes declared with the label 'var'") {
        auto entry = ParseEntry(R"(var dc.b 4/4,6,2-1)");

        WHEN("state is in psect context") {
            AssemblyState state {};
            state.in_psect = true;

            auto& data_map = state.result->psect.code_data;

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.code == 3);

            for (std::size_t i = 0; i < 3; i++) {
                REQUIRE(data_map[i].size == 1);
                REQUIRE(data_map[i].is_signed == false);
            }

            // Invoke second pass.
            for (auto& action : state.second_pass_queue2) {
                (*action)(state);
            }

            REQUIRE(state.result->psect.code_data[0].data.u8 == 1);
            REQUIRE(state.result->psect.code_data[1].data.u8 == 6);
            REQUIRE(state.result->psect.code_data[2].data.u8 == 1);

            //REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }

        WHEN("state is in a vsect and psect context") {
            AssemblyState state {};
            state.in_psect = true;
            state.in_vsect = true;

            auto& data_map = state.result->psect.initialized_data;

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.initialized_data == 3);

            for (std::size_t i = 0; i < 3; i++) {
                REQUIRE(data_map[i].size == 1);
                REQUIRE(data_map[i].is_signed == false);
            }

            // Invoke second pass.
            for (auto& action : state.second_pass_queue2) {
                (*action)(state);
            }

            REQUIRE(state.result->psect.initialized_data[0].data.u8 == 1);
            REQUIRE(state.result->psect.initialized_data[1].data.u8 == 6);
            REQUIRE(state.result->psect.initialized_data[2].data.u8 == 1);
            //REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }

        WHEN("state is in a remote vsect and psect context") {
            AssemblyState state {};
            state.in_psect = true;
            state.in_vsect = true;
            state.in_remote_vsect = true;

            auto& data_map = state.result->psect.remote_initialized_data;

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.remote_initialized_data == 3);

            for (std::size_t i = 0; i < 3; i++) {
                REQUIRE(data_map[i].size == 1);
                REQUIRE(data_map[i].is_signed == false);
            }

            // Invoke second pass.
            for (auto& action : state.second_pass_queue2) {
                (*action)(state);
            }

            REQUIRE(state.result->psect.remote_initialized_data[0].data.u8 == 1);
            REQUIRE(state.result->psect.remote_initialized_data[1].data.u8 == 6);
            REQUIRE(state.result->psect.remote_initialized_data[2].data.u8 == 1);
            //REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }
    }

    GIVEN("multiple unsigned constant words declared with the label 'var'") {
        auto entry = ParseEntry(R"(var dc.w 4/4,6,2-1)");

        WHEN("state is in psect context") {
            AssemblyState state {};
            state.in_psect = true;

            auto& data_map = state.result->psect.code_data;

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.code == 6);

            for (std::size_t i = 0; i < 6; i += 2) {
                REQUIRE(data_map[i].size == 2);
                REQUIRE(data_map[i].is_signed == false);
            }

            // Invoke second pass.
            for (auto& action : state.second_pass_queue2) {
                (*action)(state);
            }

            REQUIRE(state.result->psect.code_data[0].data.u16 == 1);
            REQUIRE(state.result->psect.code_data[2].data.u16 == 6);
            REQUIRE(state.result->psect.code_data[4].data.u16 == 1);
            //REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }

        WHEN("state is in a vsect and psect context") {
            AssemblyState state {};
            state.in_psect = true;
            state.in_vsect = true;

            auto& data_map = state.result->psect.initialized_data;

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.initialized_data == 6);

            for (std::size_t i = 0; i < 6; i += 2) {
                REQUIRE(data_map[i].size == 2);
                REQUIRE(data_map[i].is_signed == false);
            }

            // Invoke second pass.
            for (auto& action : state.second_pass_queue2) {
                (*action)(state);
            }

            REQUIRE(state.result->psect.initialized_data[0].data.u16 == 1);
            REQUIRE(state.result->psect.initialized_data[2].data.u16 == 6);
            REQUIRE(state.result->psect.initialized_data[4].data.u16 == 1);
            //REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }

        WHEN("state is in a remote vsect and psect context") {
            AssemblyState state {};
            state.in_psect = true;
            state.in_vsect = true;
            state.in_remote_vsect = true;

            auto& data_map = state.result->psect.remote_initialized_data;

            REQUIRE(handler.Handle(entry, state));
            REQUIRE(state.result->counter.remote_initialized_data == 6);

            for (std::size_t i = 0; i < 6; i += 2) {
                REQUIRE(data_map[i].size == 2);
                REQUIRE(data_map[i].is_signed == false);
            }

            // Invoke second pass.
            for (auto& action : state.second_pass_queue2) {
                (*action)(state);
            }

            REQUIRE(state.result->psect.remote_initialized_data[0].data.u16 == 1);
            REQUIRE(state.result->psect.remote_initialized_data[2].data.u16 == 6);
            REQUIRE(state.result->psect.remote_initialized_data[4].data.u16 == 1);
            //REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
        }
    }
}
}