cmake_minimum_required(VERSION 3.20)
project(toi-cplus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies via vcpkg
# Uses VCPKG_ROOT environment variable per DECISIONS.md §4
if(DEFINED ENV{VCPKG_ROOT})
    list(APPEND CMAKE_PREFIX_PATH "$ENV{VCPKG_ROOT}/installed/x64-linux")
endif()
find_package(nlohmann_json 3.4.0 REQUIRED)
find_package(Catch2 3.4.0 REQUIRED)
find_package(tl_expected 2.0.0 REQUIRED)

# TOI library
add_library(toi-cplus
    include/toi/TOITypes.h
    include/toi/TermsOfInteraction.h
    src/TermsOfInteraction.cpp
    src/TOIManager.cpp
)

set_target_properties(toi-cplus PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF)

target_link_libraries(toi-cplus PRIVATE
    nlohmann_json::nlohmann_json
    tl_expected::tl_expected)

# Include directories
target_include_directories(toi-cplus PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Tests
enable_testing()
add_executable(test_toi_manager tests/test_toi_manager.cpp)
target_link_libraries(test_toi_manager PRIVATE toi-cplus Catch2::Catch2)
add_test(NAME TOI COMMAND test_toi_manager)
