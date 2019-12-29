find_package(Catch2 REQUIRED)

add_executable(test-toolchain-libs
        test-toolchain-libs.cpp
        Assembler/ComparisonHelpers.cpp
        Assembler/PrinterHelpers.cpp
        Assembler/TestInputFileParser.cpp
        Assembler/TestExpressionLexer.cpp
        Assembler/TestExpressionParser.cpp)

target_link_libraries(test-toolchain-libs PRIVATE
        Assembler
        Module
        ROF
        Catch2::Catch2)
