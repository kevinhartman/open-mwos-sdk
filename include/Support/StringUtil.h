#pragma once

#include <string>
#include <regex>
#include <vector>

std::vector<std::string> Split(const std::string& s, std::regex delimiter) {
    std::sregex_token_iterator iter(s.begin(), s.end(), delimiter, -1);
    std::sregex_token_iterator end;

    std::vector<std::string> elements {};
    while (iter != end)  {
        elements.push_back(*iter);
        ++iter;
    }

    return elements;
}