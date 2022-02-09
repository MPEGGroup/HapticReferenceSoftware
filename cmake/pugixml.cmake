cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)
if(NO_INTERNET)
    set(LOCAL_PUGIXML_DIR ${CMAKE_SOURCE_DIR}/../pugixml CACHE PATH "Path to the local PUGIXML directory" )
    message(STATUS "Looking for a local copy of the PUGIXML test framework in ${LOCAL_PUGIXML_DIR}")
    fetchcontent_declare(DR_LIBS URL ${LOCAL_PUGIXML_DIR})
else()
        FetchContent_Declare(pugixml
            GIT_REPOSITORY "https://github.com/zeux/pugixml"
            GIT_TAG "v1.12"
            GIT_SHALLOW ON
            GIT_PROGRESS TRUE
        )
endif()
set(PUGIXML_NO_XPATH ON CACHE INTERNAL "")
FetchContent_MakeAvailable(PUGIXML)

# Make PUGIXML cmake scripts available
set(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/_deps/pugixml-src/contrib/")

include_directories(${CMAKE_BINARY_DIR}/_deps/pugixml-src/src/)
