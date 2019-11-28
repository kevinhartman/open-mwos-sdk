#pragma once

#include <string>
#include <istream>

class Parser {
public:
    Parser() = default;
    void Parse(std::istream& lines) const;
};