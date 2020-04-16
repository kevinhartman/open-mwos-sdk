#pragma once

#include <string>
#include <regex>
#include <vector>

namespace {
std::vector<std::string> Split(const std::string& s, const std::regex& delimiter) {
    std::vector<std::string> elements {};

    // sregex_token_iterator splitter usage doesn't seem to handle empty string properly.
    if (s == "") {
        return elements;
    }

    std::sregex_token_iterator iter(s.begin(), s.end(), delimiter, -1);
    std::sregex_token_iterator end;

    while (iter != end)  {
        elements.push_back(*iter);
        ++iter;
    }

    return elements;
}
}