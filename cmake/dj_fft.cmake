cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

option(BUILD_DJ_FFT "Build and use DJ_FFT" ON)

if(BUILD_DJ_FFT)
    include(FetchContent)
    if(NO_INTERNET)
        set(LOCAL_DJ_FFT_DIR ${CMAKE_SOURCE_DIR}/../dj_fft CACHE PATH "Path to the local dj_fft directory" )
        message(STATUS "Looking for a local copy of the DJ_FFT test framework in ${LOCAL_DJ_FFT_DIR}")
        fetchcontent_declare(DR_LIBS URL ${LOCAL_DJ_FFT_DIR})
    else()
        fetchcontent_declare(DJ_FFT
                GIT_REPOSITORY https://github.com/jdupuy/dj_fft.git
                #GIT_TAG "49de65c5204cb055afc1a14b11753e17d52a1d1b"
                GIT_PROGRESS TRUE
                )
    endif()

    fetchcontent_makeavailable(DJ_FFT)

    # Make DJ_FFT header files available
    include_directories(${CMAKE_BINARY_DIR}/_deps/dj_fft-src/)
endif()
