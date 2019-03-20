#!/bin/bash
#
# Adopted from:
# https://github.com/codecov/example-cpp11-cmake/blob/2036ea/build.sh

set -ex

if [ ! -z "${CLANG_FORMAT:-}" ]; then
  if [ ! -f ".clang-format" ]; then
    echo "Couldn't find .clang-format file."
    exit 1
  fi

  FILES=`find . -regex '\./\(src\|tests\)/.*\.\(cpp\|hpp\|h\)$'`
  for FILE in $FILES; do
    "$CLANG_FORMAT" -i "$FILE"
  done

  DIRTY=`git ls-files --modified`
  if [ ! -z "${DIRTY:-}" ]; then
    echo "clang-format modified some files, printing git-diff:"
    git --no-pager diff
    exit 1
  fi
fi


if [ ! -z "${BUILD:-}" ]; then

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
