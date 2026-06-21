#!/usr/bin/env bash
set -euo pipefail

files=$(git ls-files '*.cpp' '*.h')
if [ -z "$files" ]; then
  echo "No C++ files to format"
  exit 0
fi

clang-format -i $files
echo "Formatted files:"
echo "$files"
