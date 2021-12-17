project(Encoder)

add_executable(Encoder src/main.cpp src/PcmEncoder.cpp include/PcmEncoder.h)
target_link_libraries(Encoder tools)

add_executable(test_Encoder test/PcmEncoder.test.cpp src/PcmEncoder.cpp)
target_link_libraries(test_Encoder PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_Encoder)