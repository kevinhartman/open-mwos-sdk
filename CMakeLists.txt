cmake_minimum_required(VERSION 3.10)
project(os9_assembler)

set(CMAKE_CXX_STANDARD 17)

set(OS9TOOLCHAIN_MAIN_SRC_DIR     ${CMAKE_CURRENT_SOURCE_DIR}  )
set(OS9TOOLCHAIN_MAIN_INCLUDE_DIR ${OS9TOOLCHAIN_MAIN_SRC_DIR}/include )

add_subdirectory(lib)
add_subdirectory(test)

add_subdirectory(tools)
