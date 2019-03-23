#!/bin/bash

set -ex

if [ ! -z "$CLANG_FORMAT" ]; then
  CLANG_FORMAT=clang-format
fi

if [ ! -f ".clang-format" ]; then
  echo "Couldn't find .clang-format file."
  exit 1
fi

FILES=$(find . -regex '\./\(src\|tests\)/.*\.\(cpp\|hpp\|h\)$')
for FILE in $FILES; do
  "$CLANG_FORMAT" -i "$FILE"
done

DIRTY=$(git ls-files --modified)
if [ ! -z "${DIRTY}" ]; then
  echo "clang-format modified some files, printing git-diff:"
  git --no-pager diff
  exit 1
fi
