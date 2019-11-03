#pragma once

#include <vector>
#include <string>

namespace rof {

constexpr uint16_t ROFSyncBytes = 0U; //TODO: find correct value

class ROFObjectFile {
public:
    enum Type {
        Program = 1,
        Subroutine = 2,
        Multi = 3,
        Data = 4,
        ConfigStatusDescriptor = 5,

        TrapHandlerLibrary = 11,
        System = 12,
        FileManager = 13,
        DeviceDriver = 14,
        DeviceDescriptor = 15
    };

    enum Lang {
        ObjectCode = 1,
        ICode = 2,
        ICode_Pascal = 3,
        ICode_C = 4,
        ICode_Cobol = 5,
        ICode_Fortran = 6
    };

    struct VSect {
        bool isRemote;
    };

    struct PSect {
        std::string name;
        uint16_t
            tylan,
            revision,
            edition;
        uint32_t
            stack,
            entry_offset,
            trap_handler_offset;

        std::vector<VSect> vsects;

        Type GetType() {
            return static_cast<Type>(tylan >> 8U);
        }

        Lang GetLang() {
            return static_cast<Lang>(tylan & 0xFFU);
        }
    };

public:
    ROFObjectFile();

    const PSect& GetPSect() {
        return psect;
    }

    const std::vector<VSect>& GetVSects() {
        return vsects;
    }

private:
    PSect psect;
    std::vector<VSect> vsects; // Vsect outside of PSect is unlikely, but possible
};

}