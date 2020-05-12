#include <catch2/catch.hpp>

#include <Endian.h>
#include <Rof15Header.h>
#include <Rof15ObjectFile.h>
#include <Rof15ObjectWriter.h>
#include <Serialization.h>

#include <chrono>
#include <sstream>

namespace rof {

SCENARIO("ROF header is written correctly", "[serializer][rof]") {

    GIVEN("A valid ObjectFile") {
        object::ObjectFile object_file {};
        object_file.tylan = static_cast<uint16_t>(object::Type::Program)
                          | static_cast<uint16_t>(object::Lang::ObjectCode);

        object_file.revision = 1;
        object_file.assembler_version = 23;
        object_file.assembly_time_epoch = 0;
        object_file.cpu_target = object::CpuTarget::os9k_mips;
        object_file.edition = 2;
        object_file.stack_size = 4096;
        object_file.entry_offset = 0x60;
        object_file.trap_handler_offset = 0x200;
        object_file.name = "dummy";

        tm time_info;
        time_info.tm_year = 6;
        time_info.tm_mon = 5;
        time_info.tm_mday = 4;
        time_info.tm_hour = 3;
        time_info.tm_min = 2;
        time_info.tm_sec = 1;

        object_file.assembly_time = mktime(&time_info);

        WHEN("the ROF file is produced") {
            Rof15ObjectWriter writer {};

            std::stringstream buf;
            writer.Write(object_file, buf);

            // Ensure we're reading back from stream beginning
            buf.seekg (0, buf.beg);

            // Read ROF15 header
            auto header = std::make_shared<Rof15Header>();
            auto serializable = static_cast<SerializableRof15Header*>(header.get());
            serializer::Deserialize<support::Endian::big>(*serializable, buf);

            THEN("the header is correct") {
                REQUIRE(header->SyncBytes() == rof::Rof15SyncBytes);
                REQUIRE(header->TypeLanguage() == object_file.tylan);
                REQUIRE(header->Revision() == object_file.revision);
                REQUIRE(header->AsmValid() == 0);
                REQUIRE(header->AsmVersion() == object_file.assembler_version);
                REQUIRE(header->Edition() == object_file.edition);
                REQUIRE(header->StaticDataSize() == 0);
                REQUIRE(header->InitializedDataSize() == 0);
                REQUIRE(header->CodeSize() == 0);
                REQUIRE(header->RequiredStackSize() == object_file.stack_size);
                REQUIRE(header->OffsetToEntry() == object_file.entry_offset);
                REQUIRE(header->OffsetToUninitializedTrapHandler() == object_file.trap_handler_offset);
                REQUIRE(header->RemoteStaticDataSizeRequired() == 0);
                REQUIRE(header->RemoteInitializedDataSizeRequired() == 0);
                REQUIRE(header->DebugInfoSize() == 0);
                REQUIRE(header->TargetCPU() == 0x800 /* MIPS */);
                REQUIRE(header->CodeInfo() == 0);
                REQUIRE(header->Name() == object_file.name);
                REQUIRE(header->AsmDate() == std::array<uint8_t, 6>{
                    static_cast<uint8_t>(time_info.tm_year),
                    static_cast<uint8_t>(time_info.tm_mon),
                    static_cast<uint8_t>(time_info.tm_mday),
                    static_cast<uint8_t>(time_info.tm_hour),
                    static_cast<uint8_t>(time_info.tm_min),
                    static_cast<uint8_t>(time_info.tm_sec)
                });
            }
        }
    }
}

}