cmake_minimum_required(VERSION 3.13)
project(RM0)

set(CMAKE_CXX_STANDARD 17)

option(BUILD_DECODER "Enable building Decoder" ON)
option(BUILD_ENCODER "Enable building Encoder" ON)

if(BUILD_DECODER)
    add_subdirectory("RM0_Decoder")
endif()
if(BUILD_ENCODER)
    add_subdirectory("RM0_Encoder")
endif()
