#!/bin/bash
# Simple build script for the anim library

# Create build directory
mkdir -p build
cd build

# Configure using CMake
cmake ..

# Build
cmake --build .

# Run tests
ctest
