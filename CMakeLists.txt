cmake_minimum_required(VERSION 3.16)
project(automata_simulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
     "cli/*.cpp"
     "src/**/*.cpp")

add_executable(automata_simulator ${PROJECT_SOURCES})
target_include_directories(automata_simulator PRIVATE include)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "tests/**/*.cpp")
add_executable(automata_tests ${TEST_SOURCES})
target_include_directories(automata_tests PRIVATE include)
