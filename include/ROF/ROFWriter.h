#pragma once

namespace rof {

class ROFObjectFile;

class ROFWriter {
public:
    ROFWriter();

    void Write(const ROFObjectFile&);
};

}