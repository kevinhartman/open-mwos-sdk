#pragma once

#include <optional>
#include <string>

namespace assembler {

struct Label {
    std::string name;
    bool is_global;
};

struct Entry {
    std::optional <Label> label{};
    std::optional <std::string> operation{};
    std::optional <std::string> operands{};
    std::optional <std::string> comment{};
};

}