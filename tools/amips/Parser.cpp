#include "Parser.h"

#include <istream>
#include <optional>
#include <string>
#include <regex>

const std::vector<Entry>& Parser::GetListing() const {
    return listing;
}

bool Parser::ParseComment(const std::string& line, Entry& entry) const {
    const std::regex comment(R"(^\*)");

    if (std::regex_search(line, comment)) {
        entry.comment = std::regex_replace(line, comment, "");
        return true;
    }

    return false;
}

bool Parser::ParseBlank(const std::string& line) const {
    const std::regex blank(R"(\s*)");
    return std::regex_match(line, blank);
}

std::string Parser::ParseSeparator(const std::string& line) const {
    const std::regex field_separator(R"(^(\t|\ )+)");
    return std::regex_replace(line, field_separator, "");
}

std::string Parser::ParseLabel(const std::string& line, Entry& entry) const {
    const std::regex field_separator(R"(^(\t|\ )+)");
    const std::regex label_designator("^=");
    const std::regex label("^[A-Za-z@_][A-Za-z0-9@_$.]*"); // symbolic name
    const std::regex label_global_terminator("^:");

    std::string rest = line;

    // Remove label designator, if present.
    bool expect_label = false;
    if (std::regex_search(rest, label_designator)) {
        rest = std::regex_replace(rest, label_designator, "");
        expect_label = true;
    }

    std::smatch matches {};
    if (std::regex_search(rest, matches, label))
    {
        // Label detected
        Label entry_label;
        entry_label.name = matches[0];
        rest = std::regex_replace(rest, label, "");

        // Check if global
        if (std::regex_search(rest, label_global_terminator)) {
            entry_label.is_global = true;
            rest = std::regex_replace(rest, label_global_terminator, "");
        }
        else
        {
            entry_label.is_global = false;
        }

        entry.label.emplace(entry_label);
    }
    else
    {
        if (expect_label) throw InvalidLabelException("Expected valid label after label designator (=). Got: " + rest);
    }

    if (!ParseBlank(rest) && !std::regex_search(rest, field_separator)) {
        // If tokens remain on the line, an invalid label was found (we couldn't process it above).
        throw InvalidLabelException("Expected valid label. Got " + rest);
    }

    // No label.
    return rest;
}

std::string Parser::ParseOperation(const std::string& line, Entry& entry) const {
    // TODO: make sure this matches only non-space, non-special chars
    const std::regex operation(R"(^\S+)");

    std::smatch matches {};
    if (std::regex_search(line, matches, operation)) {
        entry.operation = matches[0];
        return std::regex_replace(line, operation, "");
    }

    // No operation.
    return line;
}

std::string Parser::ParseOperands(const std::string& line, Entry& entry) const {
    // TODO: make sure this matches only non-space, non-special chars
    const std::regex operands(R"(^\S+)");

    std::smatch matches {};
    if (std::regex_search(line, matches, operands)) {
        entry.operands = matches[0];
        return std::regex_replace(line, operands, "");
    }

    // No operation.
    return line;
}

void Parser::Parse(std::istream& lines) {
    std::string line;
    while (std::getline(lines, line)) {
        if (ParseBlank(line)) {
            // Skip blank line.
            continue;
        }

        Entry entry;

        if (ParseComment(line, entry)) goto done;
        line = ParseLabel(line, entry);
        line = ParseSeparator(line);

        if (ParseComment(line, entry)) goto done;
        line = ParseOperation(line, entry);
        line = ParseSeparator(line);

        if (ParseComment(line, entry)) goto done;
        line = ParseOperands(line, entry);
        line = ParseSeparator(line);

        // Finally, parse the comment, or make sure we've processed everything if it's absent.
        if (!ParseComment(line, entry) && !ParseBlank(line)) {
            // Unexpected characters remain on line.
            throw UnexpectedTokenException("Expected end of line. Instead, got: " + line);
        }

    done:
        listing.emplace_back(entry);
    }
}