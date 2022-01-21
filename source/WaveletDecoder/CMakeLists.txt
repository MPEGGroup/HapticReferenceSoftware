project(Waveletdecoder)


add_library(waveletdecoder src/WaveletDecoder.cpp include/WaveletDecoder.h)
target_link_libraries(waveletdecoder PUBLIC types filterbank)

if(BUILD_CATCH2)
    add_executable(test_waveletdecoder test/WaveletDecoder.test.cpp)
    target_link_libraries(test_waveletdecoder PRIVATE Catch2::Catch2WithMain waveletdecoder)
    catch_discover_tests(test_waveletdecoder)
endif()