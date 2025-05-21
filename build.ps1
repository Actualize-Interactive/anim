# Build script for Windows PowerShell

# Create build directory
$null = New-Item -Path .\build -ItemType Directory -Force
Set-Location -Path .\build

# Configure using CMake
# Explicitly enable tests and examples for CI/standalone builds
cmake .. -DANIM_BUILD_TESTS=ON -DANIM_BUILD_EXAMPLES=ON

# Build
cmake --build . # --verbose 

# Run tests
ctest # --verbose
