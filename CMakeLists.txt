cmake_minimum_required (VERSION 3.5)

project (white-davisbase LANGUAGES CXX VERSION 1.0.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We heavily depend on features added with C++17 (variant, optional etc.)
# GNU extensions should not be necessary.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# Set directory paths
set(DAVISBASE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(TESTS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/tests")
set(CATCH_SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/Catch2")

# Boost is necessary for parser
find_package(Boost REQUIRED)

# Fix CMP0076 issue with target_sources_local
include(cmake/target_sources_local.cmake)

# Code coverage configuration. Adopted from:
# https://github.com/codecov/example-cpp11-cmake/blob/2036ea/CMakeLists.txt
add_library(coverage_config INTERFACE)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(coverage_config INTERFACE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
endif()

option (BUILD_TESTING "Build the testing tree." ON)

# Bulds the main executable
add_executable(davisbase)
add_subdirectory("${DAVISBASE_SOURCE_DIR}")

# Enable testing if this is a root project. This project is unlikely to be
# included by others, but anyway...
if (BUILD_TESTING AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
    list(APPEND CMAKE_MODULE_PATH "${CATCH_SOURCE_DIR}/contrib")
    add_subdirectory("${CATCH_SOURCE_DIR}")

    add_executable(tests)
    add_dependencies(tests davisbase)
    add_subdirectory("${TESTS_SOURCE_DIR}")
    target_link_libraries(tests Catch2::Catch2)

    include(CTest)
    include(Catch)
    catch_discover_tests(tests)
    enable_testing()
endif()
