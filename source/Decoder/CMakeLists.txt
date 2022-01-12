project(Decoder)

add_executable(Decoder src/main.cpp)
target_link_libraries(Decoder tools types)