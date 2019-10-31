add_library(Module
        CrcGenerator.cpp
        Module.cpp
        ModuleInfoPrinter.cpp
        ModuleUtils.cpp
)

target_include_directories(Module PUBLIC
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Module
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Support
)