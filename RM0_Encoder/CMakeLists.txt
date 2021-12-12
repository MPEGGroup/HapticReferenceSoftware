project(RM0_Encoder)

add_executable(RM0_Encoder src/main.cpp src/WaveletEncoder.cpp include/WaveletEncoder.h)
target_link_libraries(RM0_Encoder tools)

add_executable(test_RM0_Encoder test/WaveletEncoder.test.cpp src/WaveletEncoder.cpp)
target_link_libraries(test_RM0_Encoder PUBLIC tools)
target_link_libraries(test_RM0_Encoder PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_RM0_Encoder)
