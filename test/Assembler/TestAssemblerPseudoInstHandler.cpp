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

    SCENARIO("DS operations work", "[assembler]") {
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
            AND_WHEN("<constant expression> bytes are declared with the label 'var'") {
                auto entry = ParseEntry("var ds.b 4+5/2");

                REQUIRE(handler.Handle(entry, state));
                REQUIRE(state.result->counter.uninitialized_data == 6);
                REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
            }
            AND_WHEN("0 bytes are declared with the label 'var'") {
                auto entry = ParseEntry("var ds.b 0");

                REQUIRE_THROWS_AS(handler.Handle(entry, state), OperandException);
            }
            AND_WHEN("<constant expression w/ extern ref> bytes are declared with the label 'var'") {
                auto entry = ParseEntry("var ds.b external+1");

                REQUIRE_THROWS_AS(handler.Handle(entry, state), OperandException);
            }
            AND_WHEN("<constant expression w/ local equ ref> bytes are declared with the label 'var'") {
                OperandInfo info {};
                info.op = "equ";
                info.operand = "5+2";

                state.psect.equs["constequ"] = std::make_unique<ExpressionOperand>(info);

                auto entry = ParseEntry("var ds.b constequ+1");
                REQUIRE(handler.Handle(entry, state));
                REQUIRE(state.result->counter.uninitialized_data == 8);
                REQUIRE(state.GetSymbol("var") == object::SymbolInfo { object::SymbolInfo::Type::UninitData, false, 0 });
            }
        }
    }
}