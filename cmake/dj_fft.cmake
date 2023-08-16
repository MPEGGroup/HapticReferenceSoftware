cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

include(FetchContent)
if(NO_INTERNET)
    set(LOCAL_DJ_FFT_DIR ${CMAKE_SOURCE_DIR}/../dj_fft CACHE PATH "Path to the local dj_fft directory" )
    message(STATUS "Looking for a local copy of the DJ_FFT test framework in ${LOCAL_DJ_FFT_DIR}")
    fetchcontent_declare(DJ_FFT URL ${LOCAL_DJ_FFT_DIR})
else()
    fetchcontent_declare(DJ_FFT
            GIT_REPOSITORY https://github.com/jdupuy/dj_fft.git
            #GIT_TAG "bce610fa485252a0a34493c4da611f095fdf6c1f"
            GIT_PROGRESS TRUE
            GIT_SHALLOW OFF
            )
endif()

fetchcontent_makeavailable(DJ_FFT)

# Make DJ_FFT header files available
include_directories(${CMAKE_BINARY_DIR}/_deps/dj_fft-src/)
