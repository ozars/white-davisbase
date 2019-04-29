cmake_minimum_required (VERSION 3.8)

project (white-davisbase LANGUAGES CXX VERSION 1.0.0)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# We heavily depend on features added with C++17 (variant, optional etc.)
# GNU extensions should not be necessary.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0")

# Output executables to bin directory
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

# Set directory paths
set(DAVISBASE_SOURCE_DIR "${PROJECT_SOURCE_DIR}/src")
set(TESTS_SOURCE_DIR "${PROJECT_SOURCE_DIR}/tests")
set(CATCH_SOURCE_DIR "${PROJECT_SOURCE_DIR}/vendor/Catch2")

# Set source file names
set(DAVISBASE_SOURCE_FILES main.cpp parser.hpp parser.cpp ast.hpp ast.cpp
    util.hpp sdl/database.cpp sdl/page.cpp sdl/table.cpp sdl/table_leaf_page.cpp
    sdl/table_interior_page.cpp sdl/column_value.cpp)
set(TESTS_SOURCE_FILES main.test.cpp parser.test.cpp sdl.test.cpp
    utility.test.hpp)

# Boost is necessary for parser
find_package(Boost REQUIRED)

# Import prepending function for prepending paths
include(cmake/prepend.cmake)

# Convert file paths to absolute paths to avoid CMP0076
prepend(DAVISBASE_SOURCE_FILES ${DAVISBASE_SOURCE_DIR} ${DAVISBASE_SOURCE_FILES})
prepend(TESTS_SOURCE_FILES ${TESTS_SOURCE_DIR} ${TESTS_SOURCE_FILES})

# Add the main executable
add_executable(davisbase ${DAVISBASE_SOURCE_FILES})

# Add testing executable if enabled
option (BUILD_TESTING "Build the testing tree." ON)
if (BUILD_TESTING AND (PROJECT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR))
  set(TESTING_ENABLED ON)
  add_executable(tests ${TESTS_SOURCE_FILES})
endif()

# Code coverage configuration. Adopted from:
# https://github.com/codecov/example-cpp11-cmake/blob/2036ea/CMakeLists.txt
add_library(coverage_config INTERFACE)

option(CODE_COVERAGE "Enable coverage reporting" OFF)
if(CODE_COVERAGE)
  if(NOT CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    message(FATAL_ERROR "g++ or clang++ is needed for coverage configuration.")
  endif()
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

  target_link_libraries(davisbase PUBLIC coverage_config)
  target_link_libraries(tests PUBLIC coverage_config)
endif()

target_include_directories(davisbase PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(davisbase PUBLIC stdc++fs ${Boost_LIBRARIES} coverage_config)

# Configure tests if enabled
if (TESTING_ENABLED)
    list(APPEND CMAKE_MODULE_PATH "${CATCH_SOURCE_DIR}/contrib")
    add_subdirectory("${CATCH_SOURCE_DIR}")

    add_dependencies(tests davisbase)

    target_include_directories(tests PRIVATE ${Boost_INCLUDE_DIRS})
    target_include_directories(tests PRIVATE "${DAVISBASE_SOURCE_DIR}")
    target_link_libraries(tests PUBLIC stdc++fs Catch2::Catch2 coverage_config)

    include(CTest)
    include(Catch)
    catch_discover_tests(tests)
    enable_testing()
endif()
