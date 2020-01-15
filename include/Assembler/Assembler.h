#pragma once

#include <vector>

namespace assembler {

class AssemblyState;
class AssemblerTarget;
class Entry;

class Assembler {

public:
    Assembler(uint16_t assembler_version, std::unique_ptr<AssemblerTarget> target);
    ~Assembler();
    void Process(const std::vector<Entry>& listing);

protected:
    bool HandleDirective(const Entry& entry, AssemblyState& state);
    bool HandlePseudoInstruction(const Entry& entry, AssemblyState& state);

private:
    uint16_t assembler_version;
    std::unique_ptr<AssemblerTarget> target;
};

}