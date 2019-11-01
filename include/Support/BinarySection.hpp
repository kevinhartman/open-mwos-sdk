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

#pragma once

#include <algorithm>
#include <cassert>

#define TARGET_BIG_ENDIAN false // Assume machine running this code is little endian

class BinarySection
{
    const char* data;
    const size_t data_size = 0;
    bool big_endian;

    size_t offset = 0;

public:
    BinarySection(const char* data, size_t data_size, bool big_endian) : data(data), data_size(data_size), big_endian(big_endian) { }
    BinarySection(const char* data, bool big_endian) : data(data), big_endian(big_endian) { }

    inline void Seek(size_t offset)
    {
        this->offset = offset;
    }
    
    template<class T>
    bool ReadNext(T* field)
    {
        if (data_size != 0 && offset + sizeof(T) > data_size) return false;

        *field = *reinterpret_cast<const T*>(data + offset);
        if (TARGET_BIG_ENDIAN != big_endian)
        {
            EndianSwap(field);
        }
        
        offset += sizeof(T);

        return true;
    }

private:
    template <class T>
    static void EndianSwap(T *field)
    {
        auto *raw = reinterpret_cast<unsigned char*>(field);
        std::reverse(raw, raw + sizeof(T));
    }
};
