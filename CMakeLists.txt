cmake_minimum_required(VERSION 3.20)
project(asfdk-cplus VERSION 1.0.3 LANGUAGES CXX)

# C++23 required: otoi-cplus exposes std::expected in its public headers
# (see DECISIONS.md §1 — std::expected is C++23; otoi port uses std::expected
#  per vORG-DEV-OTOI-1.0.3 §3 dual error-handling pattern).
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- vcpkg toolchain (DECISIONS.md §4) ---
# When building with the vcpkg toolchain, CMAKE_TOOLCHAIN_FILE is set
# automatically by the caller (e.g. `-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake`).
# When vcpkg is not available, the hermetic fallback headers in packages/include/ are used.
# The pillar CMakeLists.txt files handle find_package() / fallback include dirs.
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "vcpkg toolchain")
endif()

# --- Vendored fallback includes (DECISIONS.md §4 / packages/include/README.md) ---
# Used when vcpkg/CMake is unavailable. Pillar CMakeLists.txt files also
# add this for their standalone test runners.
add_library(vcpkg_fallback_includes INTERFACE)
target_include_directories(vcpkg_fallback_includes INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/packages/include
)

# --- Pillar packages ---
# Each pillar package's CMakeLists.txt creates its own library target.
# The umbrella (packages/asfdk) links against these targets, inheriting
# their public include directories transitively.
add_subdirectory(packages/toi)
add_subdirectory(packages/otoi)
add_subdirectory(packages/rrt-advocate)
add_subdirectory(packages/sleepwalker)

# --- ASFDK umbrella (Phase 6) ---
add_subdirectory(packages/asfdk)

# --- Enable testing at the top level ---
enable_testing()
