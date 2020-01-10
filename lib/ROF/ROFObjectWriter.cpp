#include <Serialization.h>
#include "ROFObjectWriter.h"

#include <cassert>
#include <limits>
#include <ROFObjectFile.h>

namespace rof {

namespace {

// TODO: use logic within write method and then delete.
//ROFHeader GetHeader() {
//    ROFHeader header {};
//
//    auto code_size = code.size() * sizeof(uint32_t);
//    assert(code_size <= std::numeric_limits<uint32_t>::max());
//    header.CodeSize() = code_size;
//
//    header.DebugInfoSize() = 0; // TODO: for now, this is unimplemented
//}

}

ROFObjectWriter::ROFObjectWriter(support::Endian endianness) : endianness(endianness) { }

void ROFObjectWriter::Write(const ROFObjectFile& rof_object, std::ostream& out) const {
    const auto& header = rof_object.header;

    const auto& extern_defs = rof_object.external_definitions;
    const auto& code = rof_object.code;

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

    serialize_init_data(rof_object.initialized_data);
    serialize_init_data(rof_object.remote_initialized_data);

    // TODO: debug data would be serialized here, but it's not implemented.


}

}