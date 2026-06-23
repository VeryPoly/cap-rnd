#!/usr/bin/env bash

set -euo pipefail

if ! command -v clang-format >/dev/null 2>&1
then
    echo "Error: clang-format is not installed."
    exit 1
fi

formatted=0

while IFS= read -r file
do
    clang-format -i "$file"
    echo "Formatted: $file"
    ((formatted += 1))
done < <(git ls-files '*.cpp' '*.h')

if [[ $formatted -eq 0 ]]
then
    echo "No C++ source files found."
else
    echo
    echo "Formatting completed."
    echo "Files formatted: $formatted"
fi
