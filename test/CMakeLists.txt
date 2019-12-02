find_package(Catch2 REQUIRED)

add_executable(test-toolchain-libs
        test-toolchain-libs.cpp
        Assembler/TestInputFileParser.cpp)

target_link_libraries(test-toolchain-libs PRIVATE
        Assembler
        Module
        ROF
        Catch2::Catch2)