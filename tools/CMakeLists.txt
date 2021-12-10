project(tools)

SET(CMAKE_CXX_FLAGS "-lfftw3 -L/usr/local/lib") #works for standard installation on mac; solution for windows?

add_library(tools src/InputParser.cpp src/PsychohapticModel.cpp)
#INCLUDE_DIRECTORIES(/usr/local/include)
#target_link_libraries(tools fftw3)

add_executable(test_tools src/InputParser.test.cpp src/PsychohapticModel.test.cpp)
target_link_libraries(test_tools PRIVATE Catch2::Catch2WithMain tools)
catch_discover_tests(test_tools)
