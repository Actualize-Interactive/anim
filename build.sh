#!/bin/bash
# Configure, build, and test the anim library (library + test suite).
# To build and run the examples, use examples/run_example.sh instead.
set -e

CONFIG="${1:-Release}"

# Run from the repository root regardless of where the script was invoked from.
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
cd "$SCRIPT_DIR"

# CMAKE_BUILD_TYPE covers single-config generators (Ninja/Make);
# --config covers multi-config generators (Xcode).
cmake -B build -S . -DCMAKE_BUILD_TYPE="$CONFIG" -DANIM_BUILD_TESTS=ON -DANIM_BUILD_EXAMPLES=OFF
cmake --build build --config "$CONFIG"

# -C is required for multi-config generators; without it ctest finds no tests.
ctest --test-dir build -C "$CONFIG" --output-on-failure
