#include <iostream>
#include <fstream>

#include "amips.h"

#include "ROFObjectFile.h"
#include "ROFWriter.h"
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
    rof.SetName("mockrof");
    rof.GetHeader().SyncBytes() = { 0xA, 0xB, 0xC, 0xD};
    rof.GetHeader().AsmVersion() = 2;

    rof::ROFWriter writer(constants::AssemblerVersion, support::Endian::big);
    writer.Write(rof, file);

    return 0;
}