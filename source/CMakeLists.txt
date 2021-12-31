cmake_minimum_required(VERSION 3.13)
project(CODEC)

#included here so it does not check anything else above source.
include(../cmake/clang_tidy.cmake)
include(../cmake/nlohmann_json.cmake)
include(../cmake/pugixml.cmake)

add_subdirectory("Tools")
add_subdirectory("Types")
add_subdirectory("FilterBank")
add_subdirectory("PsychohapticModel")
add_subdirectory("WaveletDecoder")

if(BUILD_DECODER)
    add_subdirectory("Decoder")
endif()
if(BUILD_ENCODER)
    add_subdirectory("Encoder")
endif()
if(BUILD_SYNTHESIZER)
    add_subdirectory("Synthesizer")
endif()
