#pragma once

#include <optional>
#include <string>
#include <istream>
#include <vector>
#include <exception>

namespace assembler {

class Entry;

class InputFileParser {
public:
    InputFileParser();
    ~InputFileParser();
    void Parse(std::istream& lines);

    const std::vector<Entry>& GetListing() const;

private:
    bool ParseBlank(const std::string& line) const;
    bool ParseComment(const std::string& line, Entry& entry) const;
    std::string ParseSeparator(const std::string& line) const;
    std::string ParseLabel(const std::string& line, Entry& entry) const;
    std::string ParseOperation(const std::string& line, Entry& entry) const;
    std::string ParseOperands(const std::string& line, Entry& entry) const;

private:
    std::vector<Entry> listing;
};

// TODO: add proper messages using some sort of string concatenation
struct InvalidLabelException : public std::runtime_error {
    explicit InvalidLabelException(const std::string& label) : runtime_error(label) {}
};

struct UnexpectedTokenException : public std::runtime_error {
    explicit UnexpectedTokenException(const std::string& token) : runtime_error(token) {}
};

}