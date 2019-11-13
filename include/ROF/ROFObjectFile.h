#pragma once

#include <vector>
#include <string>

#include "ROFHeader.h"

namespace rof {

struct ExternDefinition {
    std::string name;
    uint16_t type;
    uint32_t symbol_value;
};

class ROFObjectFile {
public:
    ROFObjectFile() = default;

    inline ROFHeader& GetHeader() {
        return header;
    }

    inline const ROFHeader& GetHeader() const {
        return header;
    }

    inline const PSect& GetPSect() const {
        return psect;
    }

    inline const std::vector<VSect>& GetVSects() const {
        return vsects;
    }

    std::vector<ExternDefinition> GetExternalDefinitions() const;

private:
    ROFHeader header;
    PSect psect;
    std::vector<VSect> vsects; // Vsect outside of PSect is unlikely, but possible
};

}