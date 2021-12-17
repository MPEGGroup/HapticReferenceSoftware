project(filterbank)

add_library(filterbank src/Filterbank.cpp src/Wavelet.cpp)

add_executable(test_filterbank test/Filterbank.test.cpp test/Wavelet.test.cpp)
target_link_libraries(test_filterbank PRIVATE Catch2::Catch2WithMain tools)

catch_discover_tests(test_filterbank)
