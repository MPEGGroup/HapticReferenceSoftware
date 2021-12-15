project(tools)

add_library(tools src/InputParser.cpp src/Wavelet.cpp src/Filterbank.cpp)
target_link_libraries(tools iir::iir)

add_executable(test_tools src/InputParser.test.cpp src/Wavelet.test.cpp src/Filterbank.test.cpp)
target_link_libraries(test_tools PRIVATE Catch2::Catch2WithMain tools)
catch_discover_tests(test_tools)
