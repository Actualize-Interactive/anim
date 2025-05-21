#!/bin/bash
# Simple build script for the anim library

# Create build directory
mkdir -p build
cd build

# Configure using CMake
# Explicitly enable tests and examples for CI/standalone builds
cmake .. -DANIM_BUILD_TESTS=ON -DANIM_BUILD_EXAMPLES=ON

# Build
cmake --build .

# Run tests
ctest
