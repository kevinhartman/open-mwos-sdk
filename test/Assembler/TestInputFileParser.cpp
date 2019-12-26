#include <catch2/catch.hpp>

#include "Endian.h"
#include "InputFileParser.h"
#include "AssemblerTypes.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <tuple>

namespace assembler {

struct LineToEntry {
    std::string input_line;
    Entry expected_entry;
};

const Entry IgnoreEntry = {};

bool operator==(const Label& l1, const Label& l2) {
    return std::tie(l1.name, l1.is_global) == std::tie(l2.name, l2.is_global);
}

bool operator==(const Entry& e1, const Entry& e2) {
    return e1.label == e2.label
        && e1.operation == e2.operation
        && e1.operands == e2.operands
        && e1.comment == e2.comment;
}

std::ostream& operator<<(std::ostream& os, const Entry& entry) {
    os << "  {" << std::endl;
    os << "    label: " << (entry.label.has_value() ? entry.label.value().name : "[nullopt]") << std::endl;
    os << "    is_global: " << (entry.label.has_value() ? (entry.label.value().is_global ? "true" : "false") : "[nullopt]") << std::endl;
    os << "    operation: " << entry.operation.value_or("[nullopt]") << std::endl;
    os << "    operands: " << entry.operands.value_or("[nullopt]") << std::endl;
    os << "    comment: " << entry.comment.value_or("[nullopt]") << std::endl;
    os << "  }";
    return os;
}

void print_listing(const std::vector<Entry>& listing) {
    std::cout << "[" << std::endl;
    for (auto& entry : listing) {
        std::cout << entry;
        std::cout << std::endl;
    }
    std::cout << "]" << std::endl;
}

SCENARIO("Input lines are properly parsed", "[parser]") {

    GIVEN("Each input line") {
        auto pair = GENERATE(values<LineToEntry>({
            // Local label, all fields provided.
            {
                "label    addiu    $1,$2,100    *add 100 to register 2 and store in 1",
                {
                    std::optional<Label>({ "label", false }),
                    "addiu",
                    "$1,$2,100",
                    "add 100 to register 2 and store in 1"
                }
            },
            // Local label, all fields provided, but with explicit label designator "=".
            {
                "=label    addiu    $1,$2,100    *add 100 to register 2 and store in 1",
                {
                    std::optional<Label>({ "label", false }),
                    "addiu",
                    "$1,$2,100",
                    "add 100 to register 2 and store in 1"
                }
            },
            // Global label, all fields provided.
            {
                "label:    addiu    $1,$2,100    *add 100 to register 2 and store in 1",
                {
                    std::optional<Label>({ "label", true }),
                    "addiu",
                    "$1,$2,100",
                    "add 100 to register 2 and store in 1"
                }
            },
            // Global label, all fields provided, but with explicit label designator "=".
            {
                "=label:    addiu    $1,$2,100    *add 100 to register 2 and store in 1",
                {
                    std::optional<Label>({ "label", true }),
                    "addiu",
                    "$1,$2,100",
                    "add 100 to register 2 and store in 1"
                }
            },
            // Full entry without a comment.
            {
                "label    addiu    $1,$2,100",
                {
                    std::optional<Label>({ "label", false }),
                    "addiu",
                    "$1,$2,100",
                    std::nullopt
                }
            },
            // Label and operation without operands or comment.
            {
                "label    nop",
                {
                    std::optional<Label>({ "label", false }),
                    "nop",
                    std::nullopt,
                    std::nullopt
                }
            },
            // Entry with label, operation, comment. No operands.
            {
                "label    nop    *do nothing",
                {
                    std::optional<Label>({ "label", false }),
                    "nop",
                    std::nullopt,
                    "do nothing"
                }
            },
            // Entry with label and comment. No operation, no operands.
            {
                "label    *just a label",
                {
                    std::optional<Label>({ "label", false }),
                    std::nullopt,
                    std::nullopt,
                    "just a label"
                }
            },
            // Entry with label only.
            {
                "label",
                {
                    std::optional<Label>({ "label", false }),
                    std::nullopt,
                    std::nullopt,
                    std::nullopt
                }
            },
            // Operation, operands, comment. No label.
            {
                "    addiu    $1,$2,100    *add 100 to register 2 and store in 1",
                {
                    std::nullopt,
                    "addiu",
                    "$1,$2,100",
                    "add 100 to register 2 and store in 1"
                }
            },
            // Operation, comment. No operands, no label.
            {
                "    nop    *add 100 to register 2 and store in 1",
                {
                    std::nullopt,
                    "nop",
                    std::nullopt,
                    "add 100 to register 2 and store in 1"
                }
            },
            // Operation. No comment, no operands, no label.
            {
                "    nop",
                {
                    std::nullopt,
                    "nop",
                    std::nullopt,
                    std::nullopt
                }
            },
            // Comment only.
            {
                "*just a comment",
                {
                    std::nullopt,
                    std::nullopt,
                    std::nullopt,
                    "just a comment"
                }
            },
            // Label starts with @.
            {
                "@label",
                {
                    std::optional<Label>({ "@label", false }),
                    std::nullopt,
                    std::nullopt,
                    std::nullopt
                }
            },
            // Label starts with _.
            {
                "_label",
                {
                    std::optional<Label>({ "_label", false }),
                    std::nullopt,
                    std::nullopt,
                    std::nullopt
                }
            },
            // Label starts with capital.
            {
                "Label",
                {
                    std::optional<Label>({ "Label", false }),
                    std::nullopt,
                    std::nullopt,
                    std::nullopt
                }
            },
            // Crazy label. Global Label with designator and body containing all characters valid from grammar.
            {
                "=@Ab$_.hello911:",
                {
                    std::optional<Label>({ "@Ab$_.hello911", true }),
                    std::nullopt,
                    std::nullopt,
                    std::nullopt
                }
            },
            // Operands with expression including * (current code loc).
            {
                "=label:    lw    $1,label-*($2)    *load word at $2 into $1",
                {
                    std::optional<Label>({ "label", true }),
                    "lw",
                    "$1,label-*($2)",
                    "load word at $2 into $1"
                }
            }
        }));

        // Prepare stream with input line.
        std::stringstream input{};
        input << pair.input_line;

        WHEN("the line is parsed") {
            InputFileParser parser{};
            parser.Parse(input);

            THEN("the corresponding entry is correct") {
                REQUIRE(parser.GetListing().size() == 1);
                REQUIRE(parser.GetListing()[0] == pair.expected_entry);
            }
        }
    }
}

SCENARIO("Invalid input lines properly fail", "[parser]") {

    GIVEN("invalid labels") {
        auto pair = GENERATE(values<LineToEntry>({
            // Label starts with digit.
            {
                "9Label",  IgnoreEntry
            },
            // Label starts with $.
            {
                "$Label",  IgnoreEntry
            },
            // Label starts with ..
            {
                ".Label",  IgnoreEntry
            },
            // Label starts with =digit.
            {
                "=9Label", IgnoreEntry
            },
            // Label starts with =$.
            {
                "=$Label", IgnoreEntry
            },
            // Label starts with =..
            {
                "=.Label", IgnoreEntry
            },
            // Label starts with invalid character.
            {
                "^Label",  IgnoreEntry
            },
            // Label body contains invalid character.
            {
                "Label^",  IgnoreEntry
            },
            // Label designator without label.
            {
                "=",       IgnoreEntry
            },
            // Label designator with empty global label.
            {
                "=:",      IgnoreEntry
            },
            // Label designator followed by invalid label.
            {
                "=label%", IgnoreEntry
            },
            // Label starts with invalid character and is only 1 character long.
            {
                "9",       IgnoreEntry
            },
            // Label with designator starts with invalid character and is only 1 character long.
            {
                "=9",      IgnoreEntry
            }
        }));

        // Prepare stream with input line.
        std::stringstream input{};
        input << pair.input_line;

        WHEN("the line is parsed") {
            InputFileParser parser{};

            THEN("invalid label exception is thrown") {
                REQUIRE_THROWS_AS(parser.Parse(input), InvalidLabelException);
            }
        }
    }

    GIVEN("lines with unexpected tokens") {
        auto pair = GENERATE(values<LineToEntry>({
            // Invalid comment (missing comment designator).
            {
                "label    addiu    $1,$2,100    add 100 to register 2 and store in 1",    IgnoreEntry
            },
            // Invalid comment on global label with designator (missing comment designator).
            {
                "=label:    addiu    $1,$2,100    add 100 to register 2 and store in 1",  IgnoreEntry
            },
            // Accidental spaces between operands.
            {
                "label    addiu    $1, $2, 100    *add 100 to register 2 and store in 1", IgnoreEntry
            },
            // No label, invalid comment (missing comment designator).
            {
                "    addiu    $1,$2,100    add 100 to register 2 and store in 1",         IgnoreEntry
            },
            // No label, accidental spaces between operands.
            {
                "    mult    $2, $3    *multiply and store in hi/lo",                     IgnoreEntry
            }
        }));

        // Prepare stream with input line.
        std::stringstream input{};
        input << pair.input_line;

        WHEN("the line is parsed") {
            InputFileParser parser{};

            THEN("unexpected token exception is thrown") {
                REQUIRE_THROWS_AS(parser.Parse(input), UnexpectedTokenException);
            }
        }
    }
}

}