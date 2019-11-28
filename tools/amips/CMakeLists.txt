add_executable(amips amips.cpp)

target_link_libraries(amips PUBLIC
        Assembler
        ROF)

add_subdirectory(test)