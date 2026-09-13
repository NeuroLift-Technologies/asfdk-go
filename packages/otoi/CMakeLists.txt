cmake_minimum_required(VERSION 3.20)
project(otoi-cplus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies via vcpkg
find_package(nlohmann_json 3.4.0 REQUIRED)
find_package(Catch2 3.4.0 REQUIRED)
find_package(spdlog 1.11.0 REQUIRED)
find_package(tl_expected 2.0.0 REQUIRED)

# OTOI library
add_library(otoi-cplus
    include/otoi/OTOITypes.h
    include/otoi/OTOIManager.h
    src/OTOIManager.cpp
)

set_target_properties(otoi-cplus PROPERTIES
    CXX_STANDARD 23
    CXX_EXTENSIONS OFF)

target_link_libraries(otoi-cplus PUBLIC
    nlohmann_json::nlohmann_json
    tl_expected::tl_expected
    spdlog::spdlog
)

target_include_directories(otoi-cplus PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Tests
enable_testing()
add_executable(test_otoi_manager tests/test_otoi_manager.cpp)
target_link_libraries(test_otoi_manager PRIVATE otoi-cplus Catch2::Catch2)
add_test(OTOI test_otoi_manager)
