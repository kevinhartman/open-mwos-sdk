#pragma once

#include "ROFHeader.h"

#include "SerializableStruct.h"
#include <Endian.h>

#include <vector>
#include <string>
#include <ostream>

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

    inline uint16_t GetCompilerVersion() const {
        return compiler_version;
    }

    inline void SetCompilerVersion(uint16_t version) {
        compiler_version = version;
    }

    inline const PSect& GetPSect() const {
        return psect;
    }

    inline const std::vector<VSect>& GetVSects() const {
        return vsects;
    }

    std::vector<ExternDefinition> GetExternalDefinitions() const;

    void Write(std::ostream&, support::Endian endianness);

private:
    uint16_t compiler_version;
    ROFHeader header;
    PSect psect;
    std::vector<VSect> vsects; // Vsect outside of PSect is unlikely, but possible
};

}