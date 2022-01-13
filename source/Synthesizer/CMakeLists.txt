project(Synthesizer)

add_executable(Synthesizer src/main.cpp src/Helper.cpp include/Helper.h)
target_link_libraries(Synthesizer PUBLIC tools types iohaptics)
target_link_libraries(Synthesizer PRIVATE nlohmann_json::nlohmann_json)

add_executable(test_Synthesizer test/Helper.test.cpp src/Helper.cpp)
target_link_libraries(test_Synthesizer PUBLIC tools types iohaptics)
target_link_libraries(test_Synthesizer PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_Synthesizer)