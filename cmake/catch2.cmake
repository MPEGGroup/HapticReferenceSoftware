cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

option(BUILD_CATCH2 "Build and use Catch2 for unit tests" ON)

if(BUILD_CATCH2)
    include(FetchContent)
    if(NO_INTERNET)
        set(LOCAL_CATCH2_DIR ${CMAKE_SOURCE_DIR}/../Catch2-2.13.4 CACHE PATH "Path to the local Catch2 directory" )
        message(STATUS "Looking for a local copy of the Catch2 test framework in ${LOCAL_CATCH2_DIR}")
        fetchcontent_declare(CATCH2 URL ${LOCAL_CATCH2_DIR})
    else()
        fetchcontent_declare(CATCH2
                GIT_REPOSITORY https://github.com/catchorg/Catch2.git
                GIT_TAG "v2.13.4"
                GIT_PROGRESS TRUE
                )
    endif()

    fetchcontent_makeavailable(CATCH2)

    # Make catch2 cmake scripts available
    set(CMAKE_MODULE_PATH
        ${CMAKE_MODULE_PATH} "${CMAKE_BINARY_DIR}/_deps/catch2-src/contrib/")

    # Make catch2 header file available
    include_directories(${CMAKE_BINARY_DIR}/_deps/catch2-src/single_include/)
endif()
