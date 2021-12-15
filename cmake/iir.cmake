cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

option(BUILD_IIR "Build and use DSP IIR" ON)

if(BUILD_IIR)
    include(FetchContent)
    if(NO_INTERNET)
        set(LOCAL_IIR_DIR ${CMAKE_SOURCE_DIR}/../iir-1.9.0 CACHE PATH "Path to the local IIR directory" )
        message(STATUS "Looking for a local copy of the IIR test framework in ${LOCAL_IIR_DIR}")
        fetchcontent_declare(IIR URL ${LOCAL_IIR_DIR})
    else()
        fetchcontent_declare(IIR
                GIT_REPOSITORY https://github.com/berndporr/iir1.git
                GIT_TAG "1.9.0"
                GIT_PROGRESS TRUE
                )
    endif()

    fetchcontent_makeavailable(IIR)

    # Make IIR header file available
    include_directories(${CMAKE_BINARY_DIR}/_deps/iir-src/)
endif()
