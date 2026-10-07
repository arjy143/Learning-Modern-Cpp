#!/usr/bin/env bash
# Usage: tests/run.sh path/to/cout2log
# Checks the preview output against expected.txt, then applies the changes,
# builds the converted program and checks it prints the same as the original.
set -euo pipefail
TOOL=$(realpath "$1")
SRC=$(cd "$(dirname "$0")" && pwd)
WORK=$(mktemp -d)
trap 'rm -rf "$WORK"' EXIT
cp "$SRC"/*.cpp "$SRC"/*.h "$WORK"
cd "$WORK"
cat > compile_commands.json <<JSON
[{"directory": "$WORK", "file": "input.cpp", "command": "c++ -std=c++20 -c input.cpp"},
 {"directory": "$WORK", "file": "warnings.cpp", "command": "c++ -std=c++20 -c warnings.cpp"}]
JSON

# 1. Preview over every file in the database.
"$TOOL" -p . --include-headers --stream=cout --stream=cerr=logging::error \
  | sed "s|$WORK/||g" > preview.txt
diff -u "$SRC/expected.txt" preview.txt
echo "preview output: OK"

# 2. Apply to input.cpp, then build and run both versions.
c++ -std=c++20 input.cpp -o original
./original > original.txt 2>&1
"$TOOL" -p . --include-headers --stream=cout --stream=cerr=logging::error \
  --apply input.cpp > /dev/null
c++ -std=c++20 input.cpp -o converted
./converted > converted.txt 2>&1
# cerr becomes logging::error, which adds an "error: " prefix.
sed -i 's/^error: to cerr/to cerr/' converted.txt
diff -u original.txt converted.txt
echo "converted program output: OK"
