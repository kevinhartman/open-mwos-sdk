#include "BinarySectionWriter.h"
#include "Endian.h"
#include "ROFHeader.h"

#include "ROFUtils.h"

namespace rof::util {

using BinarySectionWriter = support::BinarySectionWriter;
using ROFHeader = ::rof::ROFHeader;

void SerializeHeader(const ROFHeader& rof_header, support::Endian endianness, char *buffer) {
    BinarySectionWriter writer(buffer, sizeof(ROFHeader), endianness);

    writer.Write(rof_header.sync_bytes);
    writer.Write(rof_header.tylan);
    writer.Write(rof_header.revision);
    writer.Write(rof_header.asm_valid);
    writer.Write(rof_header.asm_version);
    writer.Write(rof_header.asm_date);
    writer.Write(rof_header.edition);
    writer.Write(rof_header.static_data_size);
    writer.Write(rof_header.initialized_data_size);
    writer.Write(rof_header.const_data_size);
    writer.Write(rof_header.code_size);
    writer.Write(rof_header.stack_size_required);
    writer.Write(rof_header.entry_offset);
    writer.Write(rof_header.uninitialized_trap_offset);
    writer.Write(rof_header.remote_static_data_storage_size);
    writer.Write(rof_header.remote_initialized_data_storage_size);
    writer.Write(rof_header.remote_const_data_storage_size);
    writer.Write(rof_header.debug_info_size);
    writer.Write(rof_header.target_cpu);
    writer.Write(rof_header.code_info);
    writer.Write(rof_header.header_expansion);

    // Ensure we've written the entire header.
    assert(writer.GetOffset() == sizeof(ROFHeader));
}

}