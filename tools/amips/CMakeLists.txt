add_executable(amips amips.cpp Parser.cpp)

target_link_libraries(amips PUBLIC ROF)

add_subdirectory(test)