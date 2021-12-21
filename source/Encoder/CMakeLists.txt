project(Encoder)

add_executable(Encoder src/main.cpp src/PcmEncoder.cpp include/PcmEncoder.h src/WaveletEncoder.cpp include/WaveletEncoder.h)
target_link_libraries(Encoder tools)

add_executable(test_Encoder test/PcmEncoder.test.cpp src/PcmEncoder.cpp test/WaveletEncoder.test.cpp src/WaveletEncoder.cpp)
target_link_libraries(test_Encoder PRIVATE Catch2::Catch2WithMain tools)
catch_discover_tests(test_Encoder)
