cmake_minimum_required(VERSION 3.20)
project(sleepwalker-cplus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies via vcpkg
# Uses VCPKG_ROOT environment variable per DECISIONS.md §4
if(DEFINED ENV{VCPKG_ROOT})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-linux")
endif()
find_package(nlohmann_json 3.4.0 REQUIRED)
find_package(Catch2 3.4.0 REQUIRED)
find_package(spdlog 1.11.0 REQUIRED)

# Sleepwalker library
add_library(sleepwalker-cplus
    include/sleepwalker/SleepwalkerTypes.h
    include/sleepwalker/StateDetector.h
    include/sleepwalker/ConsentManager.h
    include/sleepwalker/ContinuityManager.h
    include/sleepwalker/SleepwalkerProtocol.h
    src/StateDetector.cpp
    src/ConsentManager.cpp
    src/ContinuityManager.cpp
    src/SleepwalkerProtocol.cpp
)

set_target_properties(sleepwalker-cplus PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF)

target_link_libraries(sleepwalker-cplus PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

# Include directories
target_include_directories(sleepwalker-cplus PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ---- Testing ----
enable_testing()

# Single test executable combining all test files.
# CATCH_CONFIG_MAIN is in test_sleepwalker.cpp; the other test files
# only define TEST_CASE blocks.
add_executable(test_sleepwalker
    tests/test_state_detection.cpp
    tests/test_consent.cpp
    tests/test_continuity.cpp
    tests/test_sleepwalker.cpp
)

target_link_libraries(test_sleepwalker PRIVATE
    sleepwalker-cplus
    Catch2::Catch2WithMain
    nlohmann_json::nlohmann_json
    spdlog::spdlog
)

target_include_directories(test_sleepwalker PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Register test with CTest
add_test(NAME SleepwalkerTests COMMAND test_sleepwalker)
