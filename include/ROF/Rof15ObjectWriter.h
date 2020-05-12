#pragma once

#include <ostream>

namespace object {
    class ObjectFile;
}

namespace rof {

class Rof15ObjectWriter {
public:
    explicit Rof15ObjectWriter();
    void Write(const object::ObjectFile&, std::ostream&) const;
};

}