#include <iostream>
#include <fstream>

#include "amips.h"

#include "InputFileParser.h"
#include "ROFObjectFile.h"
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
    rof.SetCompilerVersion(constants::AssemblerVersion);
    rof.GetHeader().SyncBytes() = { 0xA, 0xB, 0xC, 0xD};
    rof.GetHeader().AsmVersion() = 2;
    rof.GetHeader().Name() = "mockrof";
    //


    std::fstream out_file;
    out_file.exceptions(std::fstream::badbit | std::fstream::failbit);

    try {
        out_file.open("amips_out.r", std::fstream::out | std::fstream::binary);
    } catch (std::exception const& e) {
        std::cerr << e.what();
        exit(1);
    }

    rof.Write(out_file, support::Endian::big);

    return 0;
}