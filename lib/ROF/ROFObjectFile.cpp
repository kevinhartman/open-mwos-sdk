#include <Serialization.h>
#include "ROFObjectFile.h"

#include <cassert>
#include <limits>
#include <ROFObjectFile.h>

namespace rof {

ROFHeader ROFObjectFile::GetHeader() const {
    ROFHeader header {};
    header.AsmVersion() = compiler_version;

    auto code_size = code.size() * sizeof(uint32_t);
    assert(code_size <= std::numeric_limits<uint32_t>::max());
    header.CodeSize() = code_size;

    header.DebugInfoSize() = 0; // TODO: for now, this is unimplemented
}

std::vector<ExternDefinition> ROFObjectFile::GetExternalDefinitions() const {
    // TODO: iterate over PSect and VSects to gather external data references.
    // Note that order should be considered (remote vs local, etc.)

    std::vector<ExternDefinition> extern_defs {};

    ExternDefinition def1 {};
    def1.Name() = "hello";
    def1.Type() = 1;
    def1.SymbolValue() = 40;

    extern_defs.emplace_back(def1);

    return extern_defs;
}

std::vector<uint32_t> ROFObjectFile::GetCode() const {
    return code;
}

std::vector<DataEntry> ROFObjectFile::GetInitializedData() const {
    return {};
}

std::vector<DataEntry> ROFObjectFile::GetRemoteInitializedData() const {
    return {};
}

bool ROFObjectFile::Validate() const {
    auto header = GetHeader();
    auto extern_defs = GetExternalDefinitions();
    auto code = GetCode();

    if (code.size() != header.CodeSize()) {
        std::cerr << "Header field CodeSize does not match actual code size." << std::endl;
        return false;
    }

    return true;
}

void ROFObjectFile::Write(std::ostream& out_stream, support::Endian endianness) const {
    auto header = GetHeader();
    auto extern_defs = GetExternalDefinitions();
    auto code = GetCode();

    assert(Validate());

    auto serialize = [&out_stream, endianness](auto data) {
        serializer::Serialize(data, out_stream, endianness);
    };

    // Write header to file
    serialize(static_cast<SerializableROFHeader>(header));

    // Write external definition section
    auto extern_defs_count = extern_defs.size();
    assert(extern_defs_count <= std::numeric_limits<uint32_t>::max());

    // External Definition Count
    serialize(static_cast<uint32_t>(extern_defs_count));

    // External Definitions
    for (const SerializableExternDefinition& extern_def : extern_defs) {
        serialize(extern_def);
    }

    // Write Code Section
    for (auto instruction : GetCode()) {
        // Note: these instructions will be endian-flipped by serializer
        serialize(instruction);
    }

    // Write Initialized Data and Initialized Remote Data Sections
    auto serialize_init_data = [&serialize](auto data) {
        for (auto data_entry : data) {
            switch (data_entry.type) {
                // Serialize the entry as the proper type (so endian correction can be done)
                case DataEntryType::Byte: serialize(data_entry.value.byte); break;
                case DataEntryType::Half: serialize(data_entry.value.half); break;
                case DataEntryType::Word: serialize(data_entry.value.word); break;
            }
        }
    };

    serialize_init_data(GetInitializedData());
    serialize_init_data(GetRemoteInitializedData());

    // TODO: debug data would be serialized here, but it's not implemented.


}

}