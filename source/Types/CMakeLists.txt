project(types)

add_library(types src/Keyframe.cpp include/Keyframe.h src/Effect.cpp include/Effect.h src/Band.cpp include/Band.h include/BandType.h include/EncodingModality.h src/Track.cpp include/Track.h src/Haptics.cpp include/Haptics.h src/Perception.cpp include/Perception.h src/Avatar.cpp include/Avatar.h src/ReferenceDevice.cpp include/ReferenceDevice.h src/IOJson.cpp include/IOJson.h src/IOBinary.cpp include/IOBinary.h)
target_link_libraries(types PRIVATE tools nlohmann_json::nlohmann_json)

add_executable(test_types test/Keyframe.test.cpp test/Effect.test.cpp test/Band.test.cpp test/Track.test.cpp test/Haptics.test.cpp test/Perception.test.cpp test/Avatar.test.cpp test/ReferenceDevice.test.cpp)

target_link_libraries(test_types PRIVATE Catch2::Catch2WithMain types tools nlohmann_json::nlohmann_json)
catch_discover_tests(test_types)