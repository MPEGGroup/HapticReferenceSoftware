cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

option(BUILD_DR_LIBS "Build and use DR_LIBS" ON)

if(BUILD_DR_LIBS)
    include(FetchContent)
    if(NO_INTERNET)
        set(LOCAL_DR_LIBS_DIR ${CMAKE_SOURCE_DIR}/../dr_libs CACHE PATH "Path to the local dr_libs directory" )
        message(STATUS "Looking for a local copy of the DR_LIBS test framework in ${LOCAL_DR_LIBS_DIR}")
        fetchcontent_declare(DR_LIBS URL ${LOCAL_DR_LIBS_DIR})
    else()
        fetchcontent_declare(DR_LIBS
                GIT_REPOSITORY https://github.com/mackron/dr_libs.git
                #GIT_TAG "1.9.0"
                GIT_PROGRESS TRUE
                )
    endif()

    fetchcontent_makeavailable(DR_LIBS)

    # Make DR_LIBS header files available
    include_directories(${CMAKE_BINARY_DIR}/_deps/dr_libs-src/)
    #add_library(dr_libs INTERFACE)
    #target_include_directories(dr_libs INTERFACE ${CMAKE_BINARY_DIR}/_deps/dr_libs-src/)
endif()
