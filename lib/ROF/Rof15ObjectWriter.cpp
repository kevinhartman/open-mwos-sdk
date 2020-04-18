#include <Serialization.h>
#include <Numeric.h>
#include <ObjectFile.h>
#include <Rof15ObjectFile.h>
#include <Rof15ObjectWriter.h>

#include <cassert>
#include <limits>
#include <utility>

namespace rof {

namespace {
void AssertValid(const object::ObjectFile& object_file) {
    if (object_file.counter.uninitialized_data > support::MaxRangeOf<decltype(std::declval<Rof15Header>().StaticDataSize())>::value)
        throw "Uninitialized counter too big for ROF.";

    if (object_file.counter.initialized_data > support::MaxRangeOf<decltype(std::declval<Rof15Header>().InitializedDataSize())>::value)
        throw "Initialized counter too big for ROF.";

    if (object_file.counter.code > support::MaxRangeOf<decltype(std::declval<Rof15Header>().CodeSize())>::value)
        throw "Code counter too big for ROF.";

    if (object_file.counter.remote_uninitialized_data > support::MaxRangeOf<decltype(std::declval<Rof15Header>().RemoteStaticDataSizeRequired())>::value)
        throw "Remote uninitialized counter too big for ROF.";

    if (object_file.counter.remote_initialized_data > support::MaxRangeOf<decltype(std::declval<Rof15Header>().RemoteInitializedDataSizeRequired())>::value)
        throw "Remote initialized counter too big for ROF.";
}

// TODO: use logic within write method and then delete.
Rof15Header GetHeader(const object::ObjectFile& object_file) {
    Rof15Header header {};
    header.TypeLanguage() = object_file.tylan;
    header.Revision() = object_file.revision;

    // TODO: write non-zero if assembly fails? Why even produce ROF?
    header.AsmValid() = 0;
    header.AsmVersion() = object_file.assembler_version;

    // TODO: For now, just 0'd. not sure the format. Could be DNP3? Convert from epoch above^
    header.AsmDate() = {0, 0, 0, 0, 0, 0};
    header.Edition() = object_file.edition;

    header.StaticDataSize() = object_file.counter.uninitialized_data;
    header.InitializedDataSize() = object_file.counter.initialized_data;
    header.CodeSize() = object_file.counter.code;
    header.RequiredStackSize() = object_file.stack_size;
    header.OffsetToEntry() = object_file.entry_offset;
    header.OffsetToUninitializedTrapHandler() = object_file.trap_handler_offset;

    header.RemoteStaticDataSizeRequired() = object_file.counter.remote_uninitialized_data;
    header.RemoteInitializedDataSizeRequired() = object_file.counter.remote_initialized_data;
    header.DebugInfoSize() = 0; // TODO: for now, this is unimplemented

    // TODO: add target CPU type here.

    // TODO: add "code" information here. i.e. flags about threading etc.

    header.Name() = object_file.name;

    return header;
}

std::vector<ExternDefinition> GetExternalDefinitions(const object::ObjectFile& object_file) {
    std::vector<ExternDefinition> extern_defs {};
    extern_defs.reserve(object_file.global_symbols.size());

    for (auto& elem : object_file.global_symbols) {
        auto& name = elem.first;
        auto& symbol = elem.second;

        //symbol.type

        rof::ExternDefinition definition {};
        definition.Name() = name;
        // TODO: finish implementing
        //definition.Type() = symbo
        //extern_definitions.emplace_back({})
    }

    return extern_defs;
}

std::vector<uint32_t> GetCode(const object::ObjectFile& object_file) {
    return {};
}

std::vector<DataEntry> GetInitializedData(const object::ObjectFile& object_file) {
    return {};
}

std::vector<DataEntry> GetRemoteInitializedData(const object::ObjectFile& object_file) {
    return {};
}
}

Rof15ObjectWriter::Rof15ObjectWriter(support::Endian endianness) : endianness(endianness) { }

void Rof15ObjectWriter::Write(const object::ObjectFile& object_file, std::ostream& out) const {
    AssertValid(object_file);

    auto header = GetHeader(object_file);

    const auto& extern_defs = GetExternalDefinitions(object_file);
    const auto& code = GetCode(object_file);

    auto serialize = [&out, this](auto data) {
        serializer::Serialize(data, out, endianness);
    };

    // Write header to file
    serialize(static_cast<SerializableRof15Header>(header));

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