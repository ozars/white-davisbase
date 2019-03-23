#!/bin/bash

set -ex

if [ ! -z "${CLANG_FORMAT}" ]; then
  MY_DIR=$(dirname "$0")
  "$MY_DIR/lint-format.sh"
fi

if [ ! -z "${BUILD}" ]; then

  if [ "$BUILD" == "debug" ]; then
    CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Debug -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DCODE_COVERAGE=ON"
  elif [ "$BUILD" == "release" ]; then
    CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Release -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON"
  fi

  # Create build directory
  mkdir -p build && cd build
  # Configure
  cmake ${CMAKE_FLAGS} ..
  # Compile
  make -j $(nproc)
  # Test
  ctest -j $(nproc) --output-on-failure
fi

if [ ! -z "$DOCKER" ]; then
  docker build -t davisbase .
  docker run -it davisbase ctest --output-on-failure
fi
