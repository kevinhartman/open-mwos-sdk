#include <iostream>
#include <fstream>

#include "amips.h"

#include "InputFileParser.h"
#include "ROFObjectFile.h"
#include "ROFObjectWriter.h"
#include "Endian.h"

int main(int argc, const char* argv[]) {
    std::fstream in_file;
    in_file.exceptions(std::fstream::badbit | std::fstream::failbit | std::fstream::eofbit);

    try {
        in_file.open(argv[1], std::fstream::in | std::fstream::binary);
    } catch (std::exception const& e) {
        std::cerr << e.what();
        exit(1);
    }

    assembler::InputFileParser parser {};
    parser.Parse(in_file);

    // create fake ROF for now
    rof::ROFObjectFile rof;
    rof.header.AsmVersion() = constants::AssemblerVersion;
    rof.header.SyncBytes() = { 0xA, 0xB, 0xC, 0xD};
    rof.header.Name() = "mockrof";
    //

    rof::ROFObjectWriter writer(support::Endian::big);

    std::fstream out_file;
    out_file.exceptions(std::fstream::badbit | std::fstream::failbit);

    try {
        out_file.open("amips_out.r", std::fstream::out | std::fstream::binary);
    } catch (std::exception const& e) {
        std::cerr << e.what();
        exit(1);
    }

    writer.Write(rof, out_file);

    return 0;
}