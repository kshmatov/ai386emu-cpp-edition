cmake_minimum_required(VERSION 3.20)

project(ai386 VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(tomlplusplus CONFIG REQUIRED)

add_library(ai386_core
  src/config/config.cpp
  src/machine/bus.cpp
  src/machine/clock.cpp
  src/machine/machine.cpp
  src/machine/memory.cpp
  src/machine/pic.cpp
  src/machine/pit.cpp
  src/machine/rtc.cpp
)
target_include_directories(ai386_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(ai386_core PUBLIC tomlplusplus::tomlplusplus)

add_executable(ai386 src/main.cpp)
target_link_libraries(ai386 PRIVATE ai386_core)

enable_testing()
find_package(GTest CONFIG REQUIRED)

add_executable(ai386_tests
  tests/config/config_test.cpp
  tests/machine/bus_test.cpp
  tests/machine/clock_test.cpp
  tests/machine/machine_test.cpp
  tests/machine/memory_test.cpp
  tests/machine/pic_test.cpp
  tests/machine/pit_test.cpp
  tests/machine/rtc_test.cpp
  tests/machine/run_loop_test.cpp
)
target_link_libraries(ai386_tests PRIVATE ai386_core GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(ai386_tests)
