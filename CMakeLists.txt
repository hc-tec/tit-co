cmake_minimum_required(VERSION 3.12)

project(tit-coroutine)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if(MSVC)
    enable_language(C CXX ASM_MASM)
else()
    enable_language(C CXX ASM)
endif()

add_compile_options(-Wall -O2 -g -Wno-sign-compare -Wno-class-memaccess -Wno-strict-aliasing)


include(FindProtobuf)
find_package(Protobuf REQUIRED)
if (Protobuf_FOUND)
    INCLUDE_DIRECTORIES(${PROTOBUF_INCLUDE_DIR})
else (Protobuf_FOUND)
    MESSAGE(FATAL_ERROR "PROTOBUF NOT FOUNED")
endif (Protobuf_FOUND)
link_directories(${PROTOBUF_LIBRARIES})

include_directories(src)


add_subdirectory(src)
add_subdirectory(test)

link_directories(third_party)

#include_directories(include/base)
#include_directories(include/log)



