find_package(Catch2 REQUIRED)
find_package(PkgConfig REQUIRED)

# Used for Assembler decode tests.
pkg_check_modules(CAPSTONE REQUIRED IMPORTED_TARGET capstone)

add_executable(test-toolchain-libs
        test-toolchain-libs.cpp
        Assembler/ComparisonHelpers.cpp
        Assembler/PrinterHelpers.cpp
        Assembler/TestInputFileParser.cpp
        Assembler/TestExpressionLexer.cpp
        Assembler/TestExpressionParser.cpp
        Assembler/TestMipsAssemblerTarget.cpp
)

target_link_libraries(test-toolchain-libs PUBLIC
        Assembler
        Module
        ROF
        Catch2::Catch2
        PkgConfig::CAPSTONE
)
