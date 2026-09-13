cmake_minimum_required(VERSION 3.20)
project(asfdk-cplus VERSION 1.0.0 LANGUAGES CXX)

# C++23 required: otoi-cplus exposes std::expected in its public headers.
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Dependencies (same vcpkg toolchain as sibling packages)
find_package(nlohmann_json 3.4.0 REQUIRED)
find_package(Catch2 3.4.0 REQUIRED)

# Umbrella library — composes the four pillar libraries.
# NOTE: pillar targets (toi-cplus, otoi-cplus, rrt-cplus, sleepwalker-cplus)
# are provided by the top-level CMakeLists.txt. Build this package as part of
# the top-level project (add_subdirectory from repo root).
add_library(asfdk-cplus
    include/asfdk/ASFDKTypes.h
    include/asfdk/ASFDK.h
    src/ASFDK.cpp
)

set_target_properties(asfdk-cplus PROPERTIES
    CXX_STANDARD 23
    CXX_EXTENSIONS OFF)

target_link_libraries(asfdk-cplus PUBLIC
    toi-cplus
    otoi-cplus
    rrt-cplus
    sleepwalker-cplus
    nlohmann_json::nlohmann_json
)

target_include_directories(asfdk-cplus PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Tests
enable_testing()

# Full suite — requires Catch2 (vcpkg toolchain).
add_executable(test_asfdk_composition tests/test_asfdk_composition.cpp)
target_link_libraries(test_asfdk_composition PRIVATE asfdk-cplus Catch2::Catch2WithMain)
add_test(NAME ASFDK_Composition COMMAND test_asfdk_composition)

# Hermetic smoke runner — dependency-free, mirrors packages/include convention.
add_executable(asfdk_standalone_test tests/standalone_test.cpp)
target_link_libraries(asfdk_standalone_test PRIVATE asfdk-cplus)
add_test(NAME ASFDK_Standalone COMMAND asfdk_standalone_test)