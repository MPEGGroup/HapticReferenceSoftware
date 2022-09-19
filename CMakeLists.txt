cmake_minimum_required(VERSION 3.13)
project(RM0)

set(CMAKE_CXX_STANDARD 17)

if (MSVC)
  add_definitions(-D_USE_MATH_DEFINES)
endif()

option(BUILD_DECODER "Enable building Decoder" ON)
option(BUILD_ENCODER "Enable building Encoder" ON)
option(BUILD_SYNTHESIZER "Enable building Synthesizer" ON)

option(NO_INTERNET "Use pre-downloaded source archives for external libraries, e.g. Catch2" OFF)

include(cmake/dr_libs.cmake)
include(cmake/dj_fft.cmake)
include(cmake/rapidjson.cmake)
include(cmake/catch2.cmake)
include(cmake/iir.cmake)
include(cmake/pugixml.cmake)

if (BUILD_CATCH2)
    include(CTest)
    include(Catch)
    enable_testing()
endif()

include_directories("source")

add_subdirectory("source")
