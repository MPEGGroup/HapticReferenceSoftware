project(Conformance)

add_executable(Conformance src/main.cpp)
target_link_libraries(Conformance PUBLIC types iohaptics)

install(TARGETS Conformance DESTINATION bin)