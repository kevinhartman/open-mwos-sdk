// Copyright 2016 Kevin Hartman
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright
//   notice, this list of conditions and the following disclaimer.
// * Redistributions in binary form must reproduce the above copyright
//   notice, this list of conditions and the following disclaimer in the
//   documentation and/or other materials provided with the distribution.
// * Neither the name of the owner nor the names of its contributors may
//   be used to endorse or promote products derived from this software
//   without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include <fstream>
#include <iostream>
#include <cassert>
#include <filesystem>

#include "Module.hpp"
#include "ModuleHeader.hpp"
#include "ModuleUtils.hpp"
#include "ModuleInfoPrinter.hpp"

#include "Serialization.h"

using namespace module;

int main(int argc, const char* argv[])
{
    std::fstream file;
    file.exceptions(std::fstream::badbit | std::fstream::failbit | std::fstream::eofbit);

    try {
        file.open(argv[1], std::fstream::in | std::fstream::binary);
    } catch (std::exception const& e) {
        std::cerr << e.what();
        exit(1);
    }

    // deserialize module header
    file.seekg(0);
    auto header = std::make_shared<ModuleHeader>();
    auto serializable = static_cast<SerializableModuleHeader*>(header.get());
    serializer::Deserialize<support::Endian::big>(*serializable, file);

    // read entire module to memory
    file.seekg(0);
    auto module_raw = std::make_unique<char[]>(header->Size()); // TODO: size could be wrong if incorrect in module
    file.read(module_raw.get(), header->Size());

    Module module(header, std::move(module_raw));
    //assert(module.IsHeaderValid());
    assert(module.IsCrcValid());

    util::PrintModuleInfo(module, std::cout);

    return 0;
}
