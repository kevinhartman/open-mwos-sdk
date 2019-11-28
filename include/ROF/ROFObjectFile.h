#pragma once

#include "ROFHeader.h"

#include "SerializableStruct.h"
#include <Endian.h>

#include <vector>
#include <string>
#include <ostream>

namespace rof {

using SerializableExternDefinition = SerializableStruct<SerializableString, uint16_t, uint32_t>;

enum ExternDefinitionType : uint16_t {
    Data_NonRemote_Uninitialized = 0b000,
    Data_NonRemote_Initialized = 0b001,
    Data_Remote_Uninitialized = 0b010,
    Data_Remote_Initialized = 0b011,

    Code_NotCommonBlock_CodeLabel = 0b100,
    Code_NotCommonBlock_SetLabel = 0b101,
    Code_NotCommonBlock_EquLabel = 0b110,

    Code_CommonBlock_CodeLabel = 0b100000100,
    Code_CommonBlock_SetLabel = 0b100000101,
    Code_CommonBlock_EquLabel = 0b100000110
};

struct ExternDefinition : SerializableExternDefinition {
    auto& Name() { return std::get<0>(*this); }
    auto& Type() { return std::get<1>(*this); }
    auto& SymbolValue() { return std::get<2>(*this); }
};

/* == not part of ROF format -- used for internal tracking */
enum DataEntryType {
    Byte,
    Half,
    Word
};

struct DataEntry {
    DataEntryType type;
    union {
        uint8_t byte;
        uint16_t half;
        uint32_t word;
    } value;
};

// ==

class ROFObjectFile {
public:
    ROFObjectFile() = default;

    ROFHeader GetHeader() const;

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

    std::vector<uint32_t> GetCode() const;

    std::vector<DataEntry> GetInitializedData() const;

    std::vector<DataEntry> GetRemoteInitializedData() const;

    bool Validate() const;
    void Write(std::ostream&, support::Endian endianness) const;

private:
    uint16_t compiler_version;
    PSect psect;
    std::vector<VSect> vsects; // Vsect outside of PSect is unlikely, but possible
    std::vector<uint32_t> code;
};

}