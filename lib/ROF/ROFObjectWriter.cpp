#include <Serialization.h>
#include <ObjectFile.h>
#include <ROFObjectFile.h>
#include "ROFObjectWriter.h"

#include <cassert>
#include <limits>

namespace rof {

namespace {

// TODO: use logic within write method and then delete.
ROFHeader GetHeader(const object::ObjectFile& object_file) {
    ROFHeader header {};
    header.Name() = object_file.name;

    // TODO: For now, just 0'd. not sure the format. Could be DNP3? Convert from epoch above^
    header.AsmDate() = {0, 0, 0, 0, 0, 0};

    // TODO: write non-zero if assembly fails? Why even produce ROF?
    header.AsmValid() = 0;

    header.TypeLanguage() = object_file.tylan;
    header.Name() = object_file.name;
    header.Edition() = object_file.edition;
    header.Revision() = object_file.revision;

    // TODO: calculate code size
    //    auto code_size = code.size() * sizeof(uint32_t);
    //    assert(code_size <= std::numeric_limits<uint32_t>::max());
    //    header.CodeSize() = code_size;

    header.DebugInfoSize() = 0; // TODO: for now, this is unimplemented

    // TODO: write the rest of the header -- not yet implemented.

// this is just here to remind me to check that header fields are big enough to hold whatever
// the data/code sizes end up being.
//    if (state.counter.uninitialized_data > support::MaxRangeOf<decltype(header.StaticDataSize())>::value) {
//        throw "Uninitialized counter got way too big!";
//    }
//    header.StaticDataSize() = state.counter.uninitialized_data;
}

std::vector<ExternDefinition> GetExternalDefinitions(const object::ObjectFile& object_file) {
    std::vector<ExternDefinition> extern_defs {};
    extern_defs.reserve(object_file.global_symbols.size());

    for (auto& elem : object_file.global_symbols) {
        auto& name = elem.first;
        auto& symbol = elem.second;

        rof::ExternDefinition definition {};
        definition.Name() = name;
        // TODO: finish implementing
        //definition.Type() = symbo
        //extern_definitions.emplace_back({})
    }

    return extern_defs;
}

std::vector<uint32_t> GetCode(const object::ObjectFile& object_file) {

}

std::vector<DataEntry> GetInitializedData(const object::ObjectFile& object_file) {

}

std::vector<DataEntry> GetRemoteInitializedData(const object::ObjectFile& object_file) {

}
}

ROFObjectWriter::ROFObjectWriter(support::Endian endianness) : endianness(endianness) { }

void ROFObjectWriter::Write(const object::ObjectFile& object_file, std::ostream& out) const {
    auto header = GetHeader(object_file);

    const auto& extern_defs = GetExternalDefinitions(object_file);
    const auto& code = GetCode(object_file);

    auto serialize = [&out, this](auto data) {
        serializer::Serialize(data, out, endianness);
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
    for (auto instruction : code) {
        // TODO: currently, the flipping below WILL cause a problem. E.g. Endian is already big coming from MIPS target.
        //       That means we should either emit host endian in targets, or never flip here (using Endian::ignore).
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

    serialize_init_data(GetInitializedData(object_file));
    serialize_init_data(GetRemoteInitializedData(object_file));

    // TODO: debug data would be serialized here, but it's not implemented.


}

}