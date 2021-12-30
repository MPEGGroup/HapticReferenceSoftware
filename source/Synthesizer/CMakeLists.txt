project(Synthesizer)

add_executable(Synthesizer src/main.cpp)
target_link_libraries(Synthesizer PUBLIC tools types)
target_link_libraries(Synthesizer PRIVATE nlohmann_json::nlohmann_json)

#add_executable(test_Synthesizer )
#target_link_libraries(test_Synthesizer PUBLIC tools types)
#target_link_libraries(test_Synthesizer PRIVATE Catch2::Catch2WithMain)
#catch_discover_tests(test_Synthesizer)