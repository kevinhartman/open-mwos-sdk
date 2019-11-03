#pragma once

#include <vector>
#include <string>

#include "ROFHeader.h"

namespace rof {

class ROFObjectFile {
public:
    ROFObjectFile();

    inline const ROFHeader& GetHeader() const {
        return header;
    }

    inline std::string GetName() const {
        return nam;
    }

    inline const PSect& GetPSect() const {
        return psect;
    }

    inline const std::vector<VSect>& GetVSects() const {
        return vsects;
    }

private:
    ROFHeader header;
    std::string nam;
    PSect psect;
    std::vector<VSect> vsects; // Vsect outside of PSect is unlikely, but possible
};

}