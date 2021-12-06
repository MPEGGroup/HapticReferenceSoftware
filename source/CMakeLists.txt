cmake_minimum_required(VERSION 3.13)
project(CODEC)

#included here so it does not check anything else above source.
include(../cmake/clang_tidy.cmake)
include(../cmake/nlohmann_json.cmake)

add_subdirectory("Tools")
add_subdirectory("Types")

if(BUILD_DECODER)
    add_subdirectory("Decoder")
endif()
if(BUILD_ENCODER)
    add_subdirectory("Encoder")
endif()
