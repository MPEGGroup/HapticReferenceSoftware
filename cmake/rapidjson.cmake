
cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)
if(NO_INTERNET)
    set(LOCAL_RAPIDJSON_DIR ${CMAKE_SOURCE_DIR}/../rapidjson CACHE PATH "Path to the local RapidJSON directory" )
    message(STATUS "Looking for a local copy of RapidJSON in ${LOCAL_RAPIDJSON_DIR}")
    FetchContent_Declare(RAPIDJSON URL ${LOCAL_RAPIDJSON_DIR})
else()
    FetchContent_Declare(RAPIDJSON
        GIT_REPOSITORY https://github.com/Tencent/rapidjson.git
        GIT_PROGRESS TRUE
        GIT_SHALLOW TRUE
        GIT_TAG master
	CONFIGURE_COMMAND ""
	BUILD_COMMAND "")
endif()

#FetchContent_MakeAvailable(RAPIDJSON)

FetchContent_GetProperties(RAPIDJSON)
if(NOT RAPIDJSON_POPULATED)
    FetchContent_Populate(RAPIDJSON)
endif()

# Make RapidJSON header files available
include_directories(${CMAKE_BINARY_DIR}/_deps/rapidjson-src/include)
