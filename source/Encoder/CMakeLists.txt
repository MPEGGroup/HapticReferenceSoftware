project(RM0_Encoder)

add_executable(RM0_Encoder src/main.cpp src/PcmEncoder.cpp include/PcmEncoder.h src/AhapEncoder.cpp include/AhapEncoder.h)
target_link_libraries(RM0_Encoder PUBLIC tools types)
target_link_libraries(RM0_Encoder PRIVATE nlohmann_json::nlohmann_json)

add_executable(test_RM0_Encoder test/PcmEncoder.test.cpp src/PcmEncoder.cpp test/AhapEncoder.test.cpp src/AhapEncoder.cpp)
target_link_libraries(test_RM0_Encoder PRIVATE Catch2::Catch2WithMain nlohmann_json::nlohmann_json)
catch_discover_tests(test_RM0_Encoder)