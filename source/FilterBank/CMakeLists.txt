project(filterbank)

add_library(filterbank src/Filterbank.cpp src/Wavelet.cpp)

if(BUILD_CATCH2)
    add_executable(test_filterbank test/Filterbank.test.cpp test/Wavelet.test.cpp)
    target_link_libraries(test_filterbank PRIVATE Catch2::Catch2WithMain filterbank iir::iir_static)
    catch_discover_tests(test_filterbank)
endif()
