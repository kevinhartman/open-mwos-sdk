add_library(ROF
        ExpressionTreeBuilder.cpp
        Rof15ObjectWriter.cpp)

target_include_directories(ROF PUBLIC
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Expression
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Object
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/ROF
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Support
)