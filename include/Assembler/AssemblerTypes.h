#pragma once

#include <optional>
#include <string>
#include <tuple>
#include <utility>

namespace assembler {

struct Label {
    std::string name {};
    bool is_global {};

    // For use with Label in std::set
    bool operator<(const Label& rhs) const
    {
        return std::tie(name, is_global) < std::tie(rhs.name, rhs.is_global);
    }
};

struct Entry {
    std::optional <Label> label{};
    std::optional <std::string> operation{};
    std::optional <std::string> operands{};
    std::optional <std::string> comment{};
};

}