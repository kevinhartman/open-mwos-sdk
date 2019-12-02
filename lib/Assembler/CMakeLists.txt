add_library(Assembler
        ExpressionParser.cpp
        InputFileParser.cpp
        ExpressionLexer.cpp)

target_include_directories(Assembler PUBLIC
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Assembler
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Support
)