#from http://mpegx.int-evry.fr/software/MPEG/MIV/RS/TM1/-/blob/main/cmake/clang_tidy.cmake

cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

option(ENABLE_CLANG_TIDY "Turn on clang_tidy processing if available" ON)

if (ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_PATH NAMES "clang-tidy")

    if(CLANG_TIDY_PATH)
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_PATH}" --extra-arg=/EHsc)
        message(STATUS "The clang-tidy path is set to ${CLANG_TIDY_PATH}")
    else()
        message(STATUS "clang-tidy could not be found")
    endif()
else()
    message(STATUS "clang-tidy is disabled")
endif()
