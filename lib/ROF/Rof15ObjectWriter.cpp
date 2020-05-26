#include <ExpressionTreeBuilder.h>
#include <Numeric.h>
#include <ObjectFile.h>
#include <Rof15ObjectFile.h>
#include <Rof15ObjectWriter.h>
#include <Serialization.h>

#include <cassert>
#include <chrono>
#include <limits>
#include <string>
#include <tuple>
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

uint16_t GetCPUIdentifier(object::CpuTarget cpu_target) {
    using namespace object;
    switch (cpu_target) {
        case CpuTarget::os9_68k: return 0x100;
        case CpuTarget::os9k_386: return 0x200;
        case CpuTarget::os9k_68k: return 0x100;
        case CpuTarget::os9k_ppc: return 0x300;
        case CpuTarget::os9k_sh5m: return 0x400;
        case CpuTarget::os9k_mips: return 0x800;
        case CpuTarget::os9k_sh: return 0x900;
        case CpuTarget::os9k_arm: return 0x500;
        case CpuTarget::os9k_sparc: return 0xA00;
        case CpuTarget::os9k_rce: return 0x700;
        case CpuTarget::os9k_sh4: return 0x901;
        case CpuTarget::os9k_armbe: return 0xB00;
        case CpuTarget::os9k_armv5: return 0xB01;
        case CpuTarget::os9k_sh4a: return 0x902;
        default: return 0;
    }
}

std::array<uint8_t, 6> GetDateTime(const std::time_t& time) {
    std::array<uint8_t, 6> result {};

    struct tm *parts = std::localtime(&time);
    result[0] = parts->tm_year;
    result[1] = parts->tm_mon;
    result[2] = parts->tm_mday;
    result[3] = parts->tm_hour;
    result[4] = parts->tm_min;
    result[5] = parts->tm_sec;

    return result;
}

// TODO: use logic within write method and then delete.
Rof15Header GetHeader(const object::ObjectFile& object_file) {
    Rof15Header header {};
    header.TypeLanguage() = object_file.tylan;
    header.Revision() = object_file.revision;

    // TODO: write non-zero if assembly fails, and user has specified 'force'
    header.AsmValid() = 0;
    header.AsmVersion() = object_file.assembler_version;

    header.AsmDate() = GetDateTime(object_file.assembly_time);
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

    header.TargetCPU() = GetCPUIdentifier(object_file.cpu_target);

    // TODO: add "code" information here. i.e. flags about threading etc.

    header.Name() = object_file.name;

    return header;
}

std::vector<ExternDefinition> GetExternalDefinitions(const object::ObjectFile& object_file) {
    std::vector<ExternDefinition> extern_defs {};

    for (auto& [name, symbol] : object_file.psect.symbols) {
        if (symbol.is_global) {
            rof::ExternDefinition definition{};
            definition.Name() = name;
            definition.SymbolValue() = symbol.value.value();
            definition.Type() = static_cast<uint16_t>(GetDefinitionType(symbol.type));

            extern_defs.emplace_back(std::move(definition));
        }
    }

    return extern_defs;
}

std::vector<uint8_t> SerializeDataMap(
    const std::map<object::local_offset, object::MemoryValue>& data_map,
    size_t size,
    support::Endian endian
) {
    std::vector<uint8_t> result {};

    size_t write = 0;
    for (auto [offset, memory] : data_map) {
        // Zero pad any memory not identified in the memory map
        result.insert(result.end(), offset - write, 0);

        if (endian != support::HostEndian) {
            std::reverse(memory.data.raw.begin(), memory.data.raw.begin() + memory.size);
        }

        result.insert(result.end(), memory.data.raw.begin(), memory.data.raw.begin() + memory.size);
        write = offset + memory.size;
    }

    // Zero pad any memory after the map, up to the final counter size
    result.insert(result.end(), size - write, 0);

    return result;
}

std::vector<uint8_t> GetCode(const object::ObjectFile& object_file) {
    return SerializeDataMap(object_file.psect.code_data, object_file.counter.code, object_file.endian);
}

std::vector<uint8_t> GetInitializedData(const object::ObjectFile& object_file) {
    return SerializeDataMap(object_file.psect.initialized_data, object_file.counter.initialized_data, object_file.endian);
}

std::vector<uint8_t> GetRemoteInitializedData(const object::ObjectFile& object_file) {
    return SerializeDataMap(object_file.psect.remote_initialized_data, object_file.counter.remote_initialized_data, object_file.endian);
}

struct ReferenceInfo {
    std::vector<Reference> references {};
    std::vector<std::unique_ptr<ExpressionTree>> trees {};
    std::vector<std::string> extern_refs {};
};

ReferenceInfo GetReferenceInfo(const object::ObjectFile& object_file) {
    ReferenceInfo reference_info {};
    auto& references = reference_info.references;
    auto& trees = reference_info.trees;
    auto& extern_refs = reference_info.extern_refs;

    auto generate_trees = [&](auto& expr) {
        auto index = trees.size();
        ExpressionTreeBuilder builder(object_file, extern_refs);
        trees.emplace_back(builder.Build(expr));
        return index;
    };

    auto generate_refs = [&](auto& data_map, ReferenceFlags flags) {
        for (auto [offset, memory] : data_map) {
            for (auto& mapping : memory.expr_mappings) {
                // TODO: check this...
                auto byte_index = (memory.size * 8 - (mapping.offset + mapping.bit_count)) / 8;

                Reference reference {};
                reference.BitNumber() = mapping.offset;
                reference.FieldLength() = mapping.bit_count;
                reference.LocalOffset() = offset + byte_index;
                reference.LocationFlag() = static_cast<uint16_t>(mapping.is_signed ? (flags | ReferenceFlags::Signed) : flags);
                reference.ExprTreeIndex() = generate_trees(*mapping.expression);

                references.emplace_back(reference);
            }
        }
    };

    generate_refs(object_file.psect.code_data, ReferenceFlags::Code);
    generate_refs(object_file.psect.initialized_data, ReferenceFlags::Data);
    generate_refs(object_file.psect.remote_initialized_data, ReferenceFlags::Data | ReferenceFlags::Remote);

    return reference_info;
}

class ExpressionTreeSerializer {
public:
    ExpressionTreeSerializer(std::ostream& out, support::Endian endian) : out(out), endian(endian) {}

    void operator()(ExpressionRef const& ref) {
        serializer::Serialize(static_cast<SerializableExprRef>(ref), out, endian);
    }

    void operator()(ExpressionVal const& val) {
        serializer::Serialize(val, out, endian);
    }

    void operator()(std::unique_ptr<ExpressionTree> const& tree) {
        if (!tree) return;
        serializer::Serialize(tree->op, out, endian);
        std::visit(ExpressionTreeSerializer(out, endian), tree->operand1);
        std::visit(ExpressionTreeSerializer(out, endian), tree->operand2);
    }

private:
    std::ostream& out;
    support::Endian endian;
};

}

Rof15ObjectWriter::Rof15ObjectWriter() = default;

void Rof15ObjectWriter::Write(const object::ObjectFile& object_file, std::ostream& out) const {
    AssertValid(object_file);

    support::Endian endian = object_file.endian;

    auto header = GetHeader(object_file);

    const auto& extern_defs = GetExternalDefinitions(object_file);
    const auto& code = GetCode(object_file);
    const auto& init_data = GetInitializedData(object_file);
    const auto& remote_init_data = GetRemoteInitializedData(object_file);

    // TODO: is this supposed to copy data? why isn't it a ref?
    auto serialize = [&out, this, endian](auto data) {
        serializer::Serialize(data, out, endian);
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
    serialize(code);

    // Write Initialized Data and Initialized Remote Data Sections
    serialize(init_data);
    serialize(remote_init_data);

    // TODO: debug data would be serialized here, but it's not implemented.

    auto [references, expression_trees, extern_refs] = GetReferenceInfo(object_file);

    // Write external ref count
    serialize(static_cast<uint32_t>(extern_refs.size()));

    // Write external references
    serialize(extern_refs);

    // Write expression tree size (TODO: check bounds)
    serialize(static_cast<uint32_t>(expression_trees.size()));

    // Write expression tree structures
    ExpressionTreeSerializer tree_serializer(out, endian);
    for (auto& tree : expression_trees) {
        tree_serializer(tree);
    }

    // Write reference count
    serialize(static_cast<uint32_t>(references.size()));

    // Write reference structures
    for (Reference ref : references) {
        serialize(static_cast<SerializableReference>(ref));
    }
}

}