#pragma once

#include "ROFHeader.h"

#include "SerializableStruct.h"

#include <vector>
#include <string>

namespace rof {

using SerializableExternDefinition = SerializableStruct<SerializableString, uint16_t, uint32_t>;

struct ExternDefinition : SerializableExternDefinition {
    auto& Name() { return std::get<0>(*this); }
    auto& Type() { return std::get<1>(*this); }
    auto& SymbolValue() { return std::get<2>(*this); }
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