#pragma once

#include <vector>

namespace assembler {

class AssemblerTarget;
class Entry;

class Assembler {

public:
    Assembler(std::unique_ptr<AssemblerTarget> target);
    ~Assembler();
    void Process(const std::vector<Entry>& listing);

private:
    std::unique_ptr<AssemblerTarget> target;
};

}