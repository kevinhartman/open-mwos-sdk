add_library(ROF
        ROFObjectFile.cpp
        ROFWriter.cpp)

target_include_directories(ROF PUBLIC
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/ROF
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Support
)