#include "ROFObjectFile.h"

namespace rof {

std::vector<ExternDefinition> ROFObjectFile::GetExternalDefinitions() const {
    // TODO: iterate over PSect and VSects to gather external data references.
    // Note that order should be considered (remote vs local, etc.)
}

}