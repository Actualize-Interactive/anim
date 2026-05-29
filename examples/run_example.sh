#!/bin/bash
# Build the project and run the curve_visualization example.
# Usage: ./run_example.sh [Release|Debug|RelWithDebInfo]   (default: Release)
set -e

CONFIG="${1:-Release}"

# Repo root is the parent of this script's directory (examples/).
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"
REPO_ROOT="$( cd "$SCRIPT_DIR/.." &> /dev/null && pwd )"
cd "$REPO_ROOT"

# CMAKE_BUILD_TYPE covers single-config generators (Make/Ninja);
# --config covers multi-config generators (Xcode).
cmake -B build -S . -DCMAKE_BUILD_TYPE="$CONFIG" -DANIM_BUILD_TESTS=ON -DANIM_BUILD_EXAMPLES=ON
cmake --build build --config "$CONFIG"

# Multi-config generators place the binary in a per-config subdirectory;
# single-config generators place it directly under build/examples.
BIN="build/examples/$CONFIG/curve_visualization"
[ -x "$BIN" ] || BIN="build/examples/curve_visualization"
"$BIN"
