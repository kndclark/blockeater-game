cmake_minimum_required(VERSION 3.16)
project(test_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp")
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Exclude the main game entry point from the test build to avoid a "multiple definition of main" error.
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../src/game.cpp")

# Find SDL2
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

# Add path for project headers and third-party libraries
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party)

# Find GoogleTest
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(test_game_test ${TEST_SOURCES} ${SOURCES})
target_link_libraries(test_game_test PRIVATE SDL2::SDL2 GTest::gtest GTest::gtest_main)

# Add a compile definition to identify test builds
target_compile_definitions(test_game_test PRIVATE IS_TEST_BUILD)

# Enable warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(test_game_test PRIVATE -Wall -Wextra -pedantic)
endif()

enable_testing()
add_test(NAME GameTest COMMAND test_game_test)