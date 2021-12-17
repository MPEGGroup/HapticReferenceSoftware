project(types)

add_library(types src/Keyframe.cpp include/Keyframe.h src/Effect.cpp include/Effect.h src/Band.cpp include/Band.h src/Track.cpp include/Track.h src/Haptics.cpp include/Haptics.h src/Perception.cpp include/Perception.h src/Avatar.cpp include/Avatar.h src/ReferenceSetup.cpp include/ReferenceSetup.h)

add_executable(test_types test/Keyframe.test.cpp test/Effect.test.cpp test/Band.test.cpp test/Track.test.cpp test/Haptics.test.cpp test/Perception.test.cpp test/Avatar.test.cpp test/ReferenceSetup.test.cpp)
target_link_libraries(test_types PRIVATE Catch2::Catch2WithMain types)
catch_discover_tests(test_types)