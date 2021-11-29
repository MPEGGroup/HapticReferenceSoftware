project(RM0_Encoder)

add_executable(RM0_Encoder src/main.cpp)
target_link_libraries(RM0_Encoder tools)

add_executable(test_RM0_Encoder test/PcmEncoder.test.cpp src/PcmEncoder.cpp)
target_link_libraries(test_RM0_Encoder PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_RM0_Encoder)