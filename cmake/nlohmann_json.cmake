
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)
if(NO_INTERNET)
    set(LOCAL_JSON_DIR ${CMAKE_SOURCE_DIR}/../nlohmann_json CACHE PATH "Path to the local nlohmannn_json directory" )
    message(STATUS "Looking for a local copy of the NLOHMANN_JSON test framework in ${LOCAL_JSON_DIR}")
    fetchcontent_declare(NLOHMANN_JSON URL ${LOCAL_JSON_DIR})
else()
  FetchContent_Declare(NLOHMANN_JSON
    GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
    GIT_PROGRESS TRUE
    GIT_SHALLOW TRUE
    GIT_TAG v3.10.5)
endif()

FetchContent_MakeAvailable(NLOHMANN_JSON)
# Make NLOHMANN_JSON header files available
include_directories(${CMAKE_BINARY_DIR}/_deps/nlohmann_json-src/single_include)
