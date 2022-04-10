cmake_minimum_required(VERSION 3.12)

project(tit-coroutine)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_compile_options(-Wall -O2 -g -Wno-sign-compare -Wno-class-memaccess -Wno-strict-aliasing)

add_subdirectory(src)
add_subdirectory(test)
