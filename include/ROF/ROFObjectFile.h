#pragma once

#include "ROFHeader.h"
#include "SerializableStruct.h"

#include <vector>

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

struct ROFObjectFile {
    ROFHeader header {};
    std::vector<ExternDefinition> external_definitions {};
    std::vector<uint32_t> code {};
    std::vector<DataEntry> initialized_data {};
    std::vector<DataEntry> remote_initialized_data {};
};

}