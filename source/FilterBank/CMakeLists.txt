project(filterbank)

add_library(filterbank src/Filterbank.cpp src/Wavelet.cpp src/FourierTools.cpp include/FourierTools.h)
target_link_libraries(filterbank iir::iir_static)

add_executable(test_filterbank test/Filterbank.test.cpp test/Wavelet.test.cpp test/FourierTools.test.cpp)
target_link_libraries(test_filterbank PRIVATE Catch2::Catch2WithMain filterbank iir::iir_static)

catch_discover_tests(test_filterbank)
