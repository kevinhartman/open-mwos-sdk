add_library(ROF
        ROFObjectWriter.cpp)

target_include_directories(ROF PUBLIC
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/ROF
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Support
)