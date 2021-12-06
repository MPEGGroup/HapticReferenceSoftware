project(types)

add_library(types src/Keyframe.cpp src/Note.cpp src/Band.cpp)

add_executable(test_types test/Keyframe.test.cpp test/Note.test.cpp test/Band.test.cpp)
target_link_libraries(test_types PRIVATE Catch2::Catch2WithMain types)
catch_discover_tests(test_types)