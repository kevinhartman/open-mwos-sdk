add_library(Assembler
        Assembler.cpp
        AssemblerDirective.cpp
        AssemblerPseudoInst.cpp
        ExpressionLexer.cpp
        ExpressionParser.cpp
        ExpressionResolver.cpp
        InputFileParser.cpp
        MipsAssemblerTarget.cpp
)

target_include_directories(Assembler PUBLIC
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Assembler
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/ROF
        ${OS9TOOLCHAIN_MAIN_INCLUDE_DIR}/Support
)