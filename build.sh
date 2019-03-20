#!/bin/bash
#
# Adopted from:
# https://github.com/codecov/example-cpp11-cmake/blob/2036ea/build.sh

set -euo pipefail

mkdir -p build && cd build

# Configure
cmake -DCMAKE_VERBOSE_MAKEFILE:BOOL=ON -DCODE_COVERAGE=ON -DCMAKE_BUILD_TYPE=Debug ..
# Build (for Make on Unix equivalent to `make -j $(nproc)`)
cmake --build . --config Debug -- -j $(nproc)
# Test
ctest -j $(nproc) --output-on-failure
