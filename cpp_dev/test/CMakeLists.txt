cmake_minimum_required(VERSION 3.16)
project(test_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add source files
file(GLOB SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/../config/*.cpp")
file(GLOB TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

# Exclude the main game entry point from the test build to avoid a "multiple definition of main" error.
list(REMOVE_ITEM SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/../src/game.cpp")

# Find SDL2 and GoogleTest
find_package(SDL2 REQUIRED)
find_package(GTest REQUIRED)

# Find SDL2_ttf using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL2_TTF REQUIRED SDL2_ttf)

# Add path for project headers and third-party libraries
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../config)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../third_party)
include_directories(${SDL2_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS} ${SDL2_TTF_INCLUDE_PATH})

add_executable(blockeater_tests ${TEST_SOURCES} ${SOURCES})
target_link_libraries(blockeater_tests PRIVATE GTest::gtest GTest::gtest_main SDL2::SDL2 ${SDL2_TTF_LIBRARIES})

# Add a compile definition to identify test builds
target_compile_definitions(blockeater_tests PRIVATE IS_TEST_BUILD)

# Enable warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(blockeater_tests PRIVATE -Wall -Wextra -pedantic)
endif()

enable_testing()
add_test(NAME GameTest COMMAND blockeater_tests)