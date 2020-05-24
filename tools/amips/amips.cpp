#include <iostream>
#include <fstream>

#include "amips.h"

#include <Assembler.h>
#include <MipsAssemblerTarget.h>
#include "InputFileParser.h"
#include "Rof15ObjectFile.h"
#include "Rof15ObjectWriter.h"
#include "Endian.h"

int main(int argc, const char* argv[]) {
    std::fstream in_file;
    in_file.exceptions(std::fstream::badbit);
    in_file.open(argv[1], std::fstream::in);

    if (!in_file.is_open()) {
        std::cerr << "Failed to open input file.";
        exit(1);
    }
    
    assembler::InputFileParser parser {};
    parser.Parse(in_file);

    auto target = std::make_unique<assembler::MipsAssemblerTarget>(support::Endian::big);
    assembler::Assembler a(constants::AssemblerVersion, std::move(target));

    auto object = a.Process(parser.GetListing());

    rof::Rof15ObjectWriter writer {};

    std::fstream out_file;
    out_file.exceptions(std::fstream::badbit | std::fstream::failbit);

    try {
        out_file.open("amips_out.r", std::fstream::out | std::fstream::binary);
    } catch (std::exception const& e) {
        std::cerr << e.what();
        exit(1);
    }

    writer.Write(*object, out_file);

    return 0;
}