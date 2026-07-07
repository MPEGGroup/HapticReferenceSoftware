
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)
if(NO_INTERNET)
    set(LOCAL_RAPIDJSON_DIR ${CMAKE_SOURCE_DIR}/../HJIF_specs CACHE PATH "Path to the local directory containing the HJIF JSON schemas" )
    message(STATUS "Looking for a local copy of HJIF JSON schema in ${LOCAL_HJIF_SPECS_DIR}")
    FetchContent_Declare(HJIF_SPECS URL ${LOCAL_HJIF_SPECS_DIR})
else()
    FetchContent_Declare(HJIF_SPECS
        GIT_REPOSITORY https://github.com/MPEGGroup/HapticJSONSchemas.git
		GIT_TAG "f01c7151a7573cba5c9c2492ef82f32ebf983462"
  		CONFIGURE_COMMAND ""
  		BUILD_COMMAND "")
endif()
FetchContent_MakeAvailable(HJIF_SPECS)
ADD_DEFINITIONS( -DJSON_SCHEMA_PATH=\"${CMAKE_BINARY_DIR}/_deps/hjif_specs-src/Specifications\")
