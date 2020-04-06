#pragma once

#include "AssemblerTarget.h"
#include "AssemblerTypes.h"
#include <Expression.h>
#include <ObjectFile.h>

#include <map>
#include <vector>

namespace assembler {
struct AssemblyState {
    bool found_program_end = false;

    bool in_psect = false;
    bool in_vsect = false;
    bool in_remote_vsect = false;
    std::vector<Label> pending_labels {};
    std::map<std::string, Label> symbol_name_to_label;

    object::ObjectFile result {};
};
}