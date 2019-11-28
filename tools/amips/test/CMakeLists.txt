find_package(Catch2 REQUIRED)

add_executable(amips-test amips-test.cpp)

target_link_libraries(amips-test PUBLIC
        Assembler
        ROF
        Catch2::Catch2)