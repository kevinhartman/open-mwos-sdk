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
    struct {
        size_t code;
        size_t initialized_data;
        size_t uninitialized_data;
        size_t remote_initialized_data;
        size_t remote_uninitialized_data;
    } counter;

    bool in_psect = false;
    bool in_vsect = false;
    bool in_remote_vsect = false;
    std::vector<Label> pending_labels {};
    std::map<std::string, Label> symbol_name_to_label;

    object::ObjectFile result {};
};
}