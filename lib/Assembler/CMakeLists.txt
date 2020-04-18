add_library(Assembler
        Assembler.cpp
        AssemblerDirectiveHandler.cpp
        AssemblerPseudoInstHandler.cpp
        ExpressionLexer.cpp
        ExpressionParser.cpp
        ExpressionResolver.cpp
        InputFileParser.cpp
        MipsAssemblerTarget.cpp
)

target_include_directories(Assembler PUBLIC
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Assembler
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Expression
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Object
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Support
)