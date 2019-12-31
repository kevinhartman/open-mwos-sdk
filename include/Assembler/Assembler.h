#pragma once

#include <vector>

namespace assembler {

class AssemblyState;
class AssemblerTarget;
class Entry;

class Assembler {

public:
    Assembler(std::unique_ptr<AssemblerTarget> target);
    ~Assembler();
    void Process(const std::vector<Entry>& listing);

protected:
    bool HandleDirective(const Entry& entry, AssemblyState& state);
    bool HandlePseudoInstruction(const Entry& entry, AssemblyState& state);

private:
    std::unique_ptr<AssemblerTarget> target;
};

}