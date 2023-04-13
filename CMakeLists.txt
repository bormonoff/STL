cmake_minimum_required(VERSION 3.11)

project(game_server CXX)
set(CMAKE_CXX_STANDARD 20)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

add_executable(hello src/main.cpp
  src/vector.h)

add_executable(vector_test src/vector.h
  tests/vector_tests.cpp)

target_include_directories(vector_test PUBLIC src)
target_link_libraries(vector_test PRIVATE CONAN_PKG::catch2) 