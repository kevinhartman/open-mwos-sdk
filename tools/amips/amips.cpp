#include <iostream>
#include <fstream>

#include "amips.h"

#include "ROFObjectFile.h"
#include "Endian.h"

int main() {
    std::fstream file;
    file.exceptions(std::fstream::badbit | std::fstream::failbit);

    try {
        file.open("amips_out.r", std::fstream::out | std::fstream::binary);
    } catch (std::exception const& e) {
        std::cerr << e.what();
        exit(1);
    }

    rof::ROFObjectFile rof;
    rof.SetCompilerVersion(constants::AssemblerVersion);
    rof.GetHeader().SyncBytes() = { 0xA, 0xB, 0xC, 0xD};
    rof.GetHeader().AsmVersion() = 2;
    rof.GetHeader().Name() = "mockrof";

    rof.Write(file, support::Endian::big);

    return 0;
}