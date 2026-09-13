cmake_minimum_required(VERSION 3.20)
project(rrt-cplus VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find dependencies via vcpkg
find_package(nlohmann_json REQUIRED)
find_package(Catch2 3 REQUIRED)
find_package(spdlog REQUIRED)

# RRT Advocate library
add_library(rrt-cplus
    include/rrt/RRTTypes.h
    include/rrt/KeywordLayer.h
    include/rrt/SentimentLayer.h
    include/rrt/BehavioralLayer.h
    include/rrt/CrisisDetector.h
    include/rrt/CrisisAssessor.h
    include/rrt/CrisisEngine.h
    include/rrt/BurnoutDetector.h
    include/rrt/RRTAdvocate.h
    src/RRTTypes.cpp
    src/KeywordLayer.cpp
    src/SentimentLayer.cpp
    src/BehavioralLayer.cpp
    src/CrisisDetector.cpp
    src/CrisisAssessor.cpp
    src/CrisisEngine.cpp
    src/BurnoutDetector.cpp
    src/RRTAdvocate.cpp
)

set_target_properties(rrt-cplus PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF)

target_link_libraries(rrt-cplus PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog)

target_include_directories(rrt-cplus PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

# Tests
enable_testing()
add_executable(test_rrt_advocate tests/test_rrt_advocate.cpp)
target_link_libraries(test_rrt_advocate PRIVATE rrt-cplus Catch2::Catch2WithMain)
add_test(RRT_Advocate test_rrt_advocate)

add_executable(test_crisis tests/test_crisis.cpp)
target_link_libraries(test_crisis PRIVATE rrt-cplus Catch2::Catch2WithMain)
add_test(Crisis test_crisis)

add_executable(test_burnout tests/test_burnout.cpp)
target_link_libraries(test_burnout PRIVATE rrt-cplus Catch2::Catch2WithMain)
add_test(Burnout test_burnout)