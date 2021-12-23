project(types)

add_library(types src/Keyframe.cpp include/Keyframe.h src/Effect.cpp include/Effect.h src/Band.cpp include/Band.h src/Track.cpp include/Track.h)

add_executable(test_types test/Keyframe.test.cpp test/Effect.test.cpp test/Band.test.cpp test/Track.test.cpp)
target_link_libraries(test_types PRIVATE Catch2::Catch2WithMain types)
catch_discover_tests(test_types)