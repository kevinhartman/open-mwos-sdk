#pragma once

#include <functional>
#include <vector>

namespace object {
    class ObjectFile;
}

namespace assembler {

class AssemblyState;
class AssemblerTarget;
class Entry;

class Assembler {

public:
    Assembler(uint16_t assembler_version, std::unique_ptr<AssemblerTarget> target);
    ~Assembler();
    std::unique_ptr<object::ObjectFile> Process(const std::vector<Entry>& listing);

protected:
    bool HandleDirective(const Entry& entry, AssemblyState& state);
    bool HandlePseudoInstruction(const Entry& entry, AssemblyState& state);
    std::unique_ptr<object::ObjectFile> CreateResult(AssemblyState& state);

private:
    uint16_t assembler_version;
    std::unique_ptr<AssemblerTarget> target;
};

}